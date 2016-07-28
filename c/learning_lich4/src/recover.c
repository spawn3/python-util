#include "config.h"

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#define DBG_SUBSYS S_LIBCLUSTER

#include "net_global.h"
#include "cluster.h"
#include "storage.h"
#include "metadata.h"
#include "ynet_rpc.h"
#include "chunk.h"
#include "adt.h"
#include "utils.h"
#include "job_dock.h"
#include "msgqueue.h"
#include "configure.h"
#include "dbg.h"

#define RECOVER_JOB_MAX 100
#define RECOVER_JOB_DEFAULT 100
#define MSGQUEUE_SIZE (1024 * 1024 * 512) //512M

#define RECOVER_PARALLEL "/dev/shm/lich4/recover/parallel"
#define RECOVER_PARALLEL_EXPDATA "/dev/shm/lich4/recover/parallel_expdata"

static int __scan_status_update(const char *name, int finished, int total)
{
        if (total == -1)
                printf("    %s\n", name);
        else
                printf("    %s (%u/%u)\n", name, finished, total);

        return 0;
}

typedef struct {
        int count;
        struct list_head list;
} recover_queue_t;

typedef struct {
        int offset;
        int size;
        struct list_head list;
} scan_queue_t;

typedef scan_entry_t entry_t;

int recover_pop(msgqueue_t *queue, int count, struct list_head *list)
{
        int ret, left, buflen, len;
        char buf[MAX_BUF_LEN];
        chkinfo_t *chkinfo;
        msgqueue_msg_t *msg;
        entry_t *ent;

        left = count;
        while (1) {
                ret = msgqueue_get(queue, buf, MAX_BUF_LEN);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }

                if (ret == 0)
                        break;

                buflen = ret;
                msg = (void*)buf;
                msg_for_each(msg, buflen) {
                        chkinfo = (void*)msg->buf;

                        ret = ymalloc((void **)&ent, sizeof(*ent) + CHKINFO_SIZE(chkinfo->repnum));
                        if (ret)
                                GOTO(err_ret, ret);

                        memcpy(ent->chkinfo, chkinfo, CHKINFO_SIZE(chkinfo->repnum));

                        list_add_tail(&ent->hook, list);

                        len = sizeof(*msg) + CHKINFO_SIZE(chkinfo->repnum);
                        ret = msgqueue_pop(queue, chkinfo, len, len);
                        if (ret < 0) {
                                ret = errno;
                                GOTO(err_ret, ret);
                        }

                        left --;
                        if (left == 0)
                                return count;
                }

        }

        return count - left;
err_ret:
        return -ret;
}

static int __recover_exec(const chkinfo_t *chkinfo, nid_t *nid, uint64_t *taskid, int flag)
{
        int ret;
        nid_t newdisk[2];

        if (chkinfo->repnum == 1) {
                /*bug 5809, need chunk_sync before chunk_move*/
                ret = chunk_sync(&chkinfo->id, nid, taskid, flag);
                if (ret)
                        GOTO(err_ret, ret);

                newdisk[0] = chkinfo->diskid[0];

                ret = dispatch_newdisk1(&newdisk[1], 1,
                                        NULL, &newdisk[0], 1, 0);
                if (ret)
                        GOTO(err_ret, ret);

                ret = chunk_move(&chkinfo->id, newdisk, 2, nid, taskid, 0, 1);
                if (ret)
                        GOTO(err_ret, ret);
        } else {
                ret = chunk_sync(&chkinfo->id, nid, taskid, flag);
                if (ret)
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

int recover_queue(msgqueue_t *queue, int total, int parallel, int *needretry, int *recover_succ)
{
        int ret, retry, finished = 0, has_cancel = 0;
        struct list_head list, newlist;
        chkinfo_t *chkinfo;
        entry_t *ent;
        struct list_head *pos, *n;
        char name[MAX_BUF_LEN], status[MAX_BUF_LEN];

        *recover_succ = 0;
        if (!msgqueue_empty(queue)) {
                *needretry = 1;
        } else
                return 0;

        INIT_LIST_HEAD(&list);
        INIT_LIST_HEAD(&newlist);
        while (1) {
                ret = recover_pop(queue, parallel, &newlist);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }

                if (ret == 0)
                        break;

                //__scan_status_update(name, finished, total);

                list_for_each_safe(pos, n, &newlist) {
                        ent = (void *)pos;
                        list_del(&ent->hook);
                        chkinfo =  ent->chkinfo;

                        DBUG("sync %s\n", CHKID_ARG(&chkinfo->id));

                        retry = 0;
                        ret = __recover_exec(chkinfo, &ent->master, &ent->taskid, 0);
                        if (ret) {
                                if (ret == EAGAIN || ret == ENONET || ret == ENOSPC) {
                                        if (retry < 30) {
                                                sleep(1);
                                                retry++;
                                                continue;
                                        } else {
                                                DWARN("sync %s fail\n",
                                                      CHKID_ARG(&chkinfo->id));
                                                *needretry = 1; 
                                                yfree((void **)&ent);
                                        }
                                } else {
                                        if (ret == ECANCELED) {
                                                has_cancel = 1;
                                        }

                                        DWARN("sync %s fail\n",
                                              CHKID_ARG(&chkinfo->id));
                                        *needretry = 1; 
                                        yfree((void **)&ent);
                                }
                        } else {
                                list_add_tail(&ent->hook, &list);
                        }
                }

                list_for_each_safe(pos, n, &list) {
                        ent = (void *)pos;
                        list_del(&ent->hook);
                        chkinfo = ent->chkinfo;

                        snprintf(name, MAX_PATH_LEN, "sync %s", CHKID_ARG(&chkinfo->id));
                        __scan_status_update(name, finished, total);

                        ret = cluster_longtaskwait(&ent->master, ent->taskid, status);
                        if (ret) {
                                if (ret == ECANCELED) {
                                        has_cancel = 1;
                                }

                                printf("\x1b[1;31m    error:sync %s @ %s fail, ret (%d) %s\x1b[0m\n",
                                       CHKID_ARG(&chkinfo->id), network_rname(&ent->master), ret, strerror(ret));
                                *needretry = 1;
                                continue;
                        }

                        finished++;

                        DBUG("sync %s finished\n", CHKID_ARG(&chkinfo->id));
                        yfree((void **)&ent);
                }
        }

        *recover_succ = finished;

        if (has_cancel) {
                ret = EAGAIN;
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        list_for_each_safe(pos, n, &newlist) {
                list_del(pos);
                yfree((void **)&pos);
        }
        list_for_each_safe(pos, n, &list) {
                list_del(pos);
                yfree((void **)&pos);
        }
        return ret;
}

int recover_replica_pop(msgqueue_t *queue, int count, struct list_head *list)
{
        return recover_pop(queue, count, list);
}

static int __recover_replica_chkinfo(const chkinfo_t *chkinfo, diskid_t disks[], int *_disks_count)
{
        int ret, i, disks_count, skip_count, repnum;
        char buf[MAX_BUF_LEN];
        nid_t skip[LICH_REPLICA_MAX];
        oinfo_t *oinfo;
        oinfo = (void *)buf;

        if (chkinfo->id.type == __META_CHUNK__
                        || chkinfo->id.type == __NODE_CHUNK__
                        || chkinfo->id.type == __ROOT_CHUNK__
                        || chkinfo->id.type == __NAMESPACE_CHUNK__
                        || chkinfo->id.type == __BUCKET_CHUNK__ ) {
                repnum = LICH_REPLICA_METADATA;
        } else if (chkinfo->id.type == __OBJECT_CHUNK__ ) {
                ret = object_getattr(&chkinfo->id, oinfo, 1);
                if (ret)
                        GOTO(err_ret, ret);

                repnum = oinfo->repnum;
        } else if (chkinfo->id.type == __RAW_CHUNK__) {
                repnum = 2;
        } else {
                YASSERT(0);
        }

        memcpy(skip, chkinfo->diskid, sizeof(nid_t) * chkinfo->repnum);
        memcpy(disks, chkinfo->diskid, sizeof(nid_t) * chkinfo->repnum);
        skip_count = chkinfo->repnum;
        disks_count = chkinfo->repnum;
        for (i = disks_count; i < repnum; i++) {
                ret = dispatch_newdisk1(&disks[disks_count],
                                1, NULL, skip, skip_count, 0);
                if (ret) {
                        if (ret == ENOSPC) {
                                break;
                        } else {
                                GOTO(err_ret, ret);
                        }
                }

                skip[skip_count] = disks[disks_count];
                skip_count++;
                disks_count++;
        }
        metadata_check_diskid(disks, disks_count);
        *_disks_count = disks_count;

        return 0;
err_ret:
        return ret;
}

int recover_replica_queue(msgqueue_t *queue, int total, int _parallel, int *needretry)
{
        int ret, retry, disks_count, finished = 0;
        int parallel, expdata, parallel_real;
        struct list_head list, newlist;
        chkinfo_t *chkinfo;
        entry_t *ent;
        struct list_head *pos, *n;
        char name[MAX_NAME_LEN], status[MAX_BUF_LEN];
        diskid_t disks[LICH_REPLICA_MAX];

        (void) total;
        (void) _parallel;

        if (!msgqueue_empty(queue)) {
                *needretry = 1;
        } else
                return 0;

        INIT_LIST_HEAD(&list);
        INIT_LIST_HEAD(&newlist);
        while (1) {
                ret = recover_parallel_get(&parallel, &expdata, &parallel_real);
                if (ret) {
                        GOTO(err_ret, ret);
                }

                printf("recover parallel: %d\n", parallel_real);
                ret = recover_replica_pop(queue, parallel_real, &newlist);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }

                if (ret == 0) {
                        break;
                }

                list_for_each_safe(pos, n, &newlist) {
                        ent = (void *)pos;
                        list_del(&ent->hook);
                        chkinfo =  ent->chkinfo;

                        DBUG("move %s\n", CHKID_ARG(&chkinfo->id));

                        ret = __recover_replica_chkinfo(chkinfo, disks, &disks_count);
                        if (ret) {
                                GOTO(err_ret, ret);
                        }

                        DINFO("move %s disks_count:%d\n",
                                     CHKID_ARG(&chkinfo->id), disks_count);

                        retry = 0;
                        ret = chunk_move(&chkinfo->id, disks,
                                        disks_count, &ent->master,
                                        &ent->taskid, 0, 1);

                        if (ret) {
                                if (ret == EAGAIN || ret == ENONET || ret == ENOSPC) {
                                        if (retry < 30) {
                                                sleep(1);
                                                retry++;
                                                continue;
                                        } else {
                                                DWARN("move %s fail\n",
                                                      CHKID_ARG(&chkinfo->id));
                                                *needretry = 1;
                                                yfree((void **)&ent);
                                        }
                                } else {
                                        DWARN("move %s fail\n",
                                              CHKID_ARG(&chkinfo->id));
                                        *needretry = 1;
                                        yfree((void **)&ent);
                                }
                        } else {
                                list_add_tail(&ent->hook, &list);
                        }
                }

                list_for_each_safe(pos, n, &list) {
                        ent = (void *)pos;
                        list_del(&ent->hook);
                        chkinfo =  ent->chkinfo;

                        snprintf(name, MAX_PATH_LEN, "move %s", CHKID_ARG(&chkinfo->id));

                        if ((LLU)ent->taskid == (LLU)-1) {
                                continue;
                        }

                        ret = cluster_longtaskwait(&ent->master, ent->taskid, status);
                        if (ret) {
                                printf("\x1b[1;31m    error:move %s @ %s fail, ret (%d) %s\x1b[0m\n",
                                       CHKID_ARG(&chkinfo->id), network_rname(&ent->master), ret, strerror(ret));
                                *needretry = 1;
                                continue;
                        }

                        finished++;

                        DINFO("move %s finished\n", CHKID_ARG(&chkinfo->id));
                        yfree((void **)&ent);
                }
        }

        return 0;
err_ret:
        list_for_each_safe(pos, n, &newlist) {
                list_del(pos);
                yfree((void **)&pos);
        }
        list_for_each_safe(pos, n, &list) {
                list_del(pos);
                yfree((void **)&pos);
        }
        return ret;
}

int recover_chkinfo_losted(const chkinfo_t *chkinfo)
{
        int ret, i, online, needcheck, available, count;
        const diskid_t *diskid;
        time_t now;

        DINFO("scan %s\n", CHKID_ARG(&chkinfo->id));

        if (chkinfo->repbakup) {
                now = gettime();
                if (now - chkinfo->mtime < gloconf.move_interval && now > chkinfo->mtime) {
                        printf("%s may be moving, check it after %u\n",
                               CHKID_ARG(&chkinfo->id), (int)(gloconf.move_interval - (now - chkinfo->mtime)));
                } else {
                        printf("needrecover, %s moving timeout %d, force check\n",
                               CHKID_ARG(&chkinfo->id), (int)((now - chkinfo->mtime) - gloconf.move_interval));

                        return 1;
                }
        } else if (chkinfo->repnum == 1) {
                printf("\x1b[1;31mneedrecover, danger %s\x1b[0m\n", CHKID_ARG(&chkinfo->id));
                return 1;
        } else {
                count = chkinfo->repnum;
                available = count;
                online = 0;
                needcheck = 0;
                for (i = 0; i < count; i++) {
                        diskid = &chkinfo->diskid[i];

                        ret = network_connect1(diskid);
                        if (ret) {
                                DINFO("ret %u\n", ret);
                                available--;
                                continue;
                        }

                        online++;

                        if (diskid->status == __S_DIRTY || diskid->status == __S_REDIRECT) {
                                needcheck++;
                                available--;
                                continue;
                        }
                }

                if (available != count) {
                        if (available == 0) {
                                printf("\x1b[1;31mneedrecover, lost %s\x1b[0m\n", CHKID_ARG(&chkinfo->id));
                        } else {
                                printf("needrecover, check %s rep %u online %u need check %u\n",
                                                CHKID_ARG(&chkinfo->id), count, online, needcheck);
                        }

                        return 1;
                }

        }

        return 0;
}

int recover_parallel_get(int *_parallel, int *_expdata, int *_parallel_real)
{
        int ret, parallel_default;

        parallel_default = RECOVER_JOB_DEFAULT ;
        ret = parallel_get(_parallel, RECOVER_PARALLEL, _expdata,
                        RECOVER_PARALLEL_EXPDATA, _parallel_real, parallel_default);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}
