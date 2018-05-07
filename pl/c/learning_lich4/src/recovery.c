#include "config.h"

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#define DBG_SUBSYS S_LIBCLUSTER

#include "net_global.h"
#include "cluster.h"
#include "storage.h"
#include "metadata.h"
#include "lich_md.h"
#include "lichstor.h"
#include "lichstor.h"
#include "ynet_rpc.h"
#include "chunk.h"
#include "adt.h"
#include "utils.h"
#include "job_dock.h"
#include "../../storage/storage/stor_rpc.h"
#include "../../storage/controller/volume_proto.h"
#include "../../storage/storage/md_parent.h"
#include "msgqueue.h"
#include "utils.h"
#include "configure.h"
#include "dbg.h"
#include "../../storage/replica/replica.h"

static int __fail__ = 0;
static int __recovery__ = 0;
static int __deep__ = 0;
static fileid_t unlink_fid;    //skip /system/unlink directory

static int __disk_online(const chkinfo_t *chkinfo, int repnum, int *online);
int disk_online(const chkinfo_t *chkinfo, int repnum);

#if 0
typedef struct {
        int needretry;
        int parallels;
        char status[MAX_NAME_LEN];
} scan_t;

typedef scan_entry_t lost_entry_t;

scan_t scan = {0, 5, {0x0}};
static int __verbose;
static int __red__ = 0;

#define RECOVER_JOB_MAX 100
#define MSGQUEUE_SIZE (1024 * 1024 * 512) //512M

#if 1

static hashtable_t table = NULL;

typedef struct {
        nid_t nid;
        uint64_t lost;
        uint64_t dirty;
} entry_t;


static uint32_t __key(const void *args)
{
        return ((nid_t *)args)->id;
}

static int __cmp(const void *v1, const void *v2)
{
        const entry_t *ent = (entry_t *)v1;

        //DBUG("cmp %s %s\n", ent->key, (const char *)v2);

        return nid_cmp(&ent->nid, (const nid_t *)v2);
}

static int __lost_table_init()
{
        int ret;

        table = hash_create_table(__cmp, __key, "lost table");
        if (table == NULL) {
                ret = ENOMEM;
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

#define SYNC_STAT_LOST 1
#define SYNC_STAT_DIRTY 2

static void __lost_table_update(const nid_t *nid, int stat)
{
        int ret;
        entry_t *ent;

        ent = hash_table_find(table, nid);
        if (ent == NULL) {
                ret = ymalloc((void **)&ent, sizeof(*ent));
                if (ret)
                        UNIMPLEMENTED(__DUMP__);

                ent->nid = *nid;
                ent->lost = 0;
                ent->dirty = 0;

                ret = hash_table_insert(table, (void *)ent, &ent->nid, 0);
                if (ret)
                        UNIMPLEMENTED(__DUMP__);
        }


        if (stat == SYNC_STAT_LOST)
                ent->lost++;
        else
                ent->dirty++;
}

static int __lost_dump_lost(void *_arg, void *_ent)
{
        entry_t *ent = _ent;

        (void) _arg;

        if (ent->lost) {
                if (__red__)
                        printf("        \x1b[1;31m* %s : lost %llu\x1b[0m\n",
                               network_rname(&ent->nid), (LLU)ent->lost);
                else
                        printf("         %s : lost %llu\n", network_rname(&ent->nid), (LLU)ent->lost);
        }

        return 0;
}

static int __lost_dump_dirty(void *_arg, void *_ent)
{
        entry_t *ent = _ent;

        (void) _arg;

        if (ent->dirty) {
                if (__red__)
                        printf("        \x1b[1;31m* %s : dirty %llu\x1b[0m\n",
                               network_rname(&ent->nid), (LLU)ent->dirty);
                else
                        printf("         %s : dirty %llu\n", network_rname(&ent->nid), (LLU)ent->dirty);
        }

        return 0;
}

static void __lost_table_dump()
{
        hash_iterate_table_entries(table, __lost_dump_dirty, NULL);
        hash_iterate_table_entries(table, __lost_dump_lost, NULL);
}

static void __scan_analysis_check(const chkinfo_t *chkinfo, int stat)
{
        int i, ret;
        const diskid_t *diskid;

        for (i = 0; i < chkinfo->repnum; i++) {
                diskid = &chkinfo->diskid[i];
                ret = network_connect1(diskid);
                if (ret) {
                        __lost_table_update(diskid, stat);
                } else {
                        if (diskid->status == __S_DIRTY || diskid->status == __S_REDIRECT) {
                                __lost_table_update(diskid, SYNC_STAT_DIRTY);
                        }
                }
        }
}

static int __scan_analysis(msgqueue_t *queue, int stat)
{
        int ret;
        struct list_head list, newlist;
        chkinfo_t *chkinfo;
        lost_entry_t *ent;
        struct list_head *pos, *n;

        if (msgqueue_empty(queue))
                return 0;

        __lost_table_init();

        INIT_LIST_HEAD(&list);
        INIT_LIST_HEAD(&newlist);
        while (1) {
                ret = recover_pop(queue, 100, &newlist);
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

                        __scan_analysis_check(chkinfo, stat);

                        yfree((void **)&pos);
                }
        }

        __lost_table_dump();

        return 0;
err_ret:
        return ret;
}
#endif

#endif

static int __recovery_needcheck(const chkinfo_t *chkinfo)
{
        int ret, i;

        for (i = 0; i < chkinfo->repnum; i++) {
                if (chkinfo->diskid[i].status)
                        return 1;

                ret = network_connect(&chkinfo->diskid[i].id, NULL, 3, 0);
                if (ret) {
                        return 1;
                }

                if (!disk_online(chkinfo, i))
                        return 1;
        }

        return 0;
}

static int __recovery_chunk(const nid_t *parentnid,
                            const fileid_t *parent, const chkinfo_t *chkinfo)
{
        int ret;//, retry = 0;
        char buf[MAX_BUF_LEN];

        CHKINFO_STR(chkinfo, buf);
        if (__recovery_needcheck(chkinfo) || __deep__) {
                printf("recovery: %s\n", buf);

                if (__recovery__) {
                        ret = md_chunk_check(parentnid, parent, &chkinfo->id, -1, 0);
                        if (ret)
                                GOTO(err_ret, ret);
                }
        } else {
                printf("check: %s\n", buf);
        }

        return 0;
err_ret:
        return ret;
}

static void __recovery_file_range__(const nid_t *nid, const fileid_t *fileid, int from, int to, int *_fail)
{
        int ret, i, fail = 0;
        chkid_t chkid, _from, _to;
        char buf[MAX_BUF_LEN];
        chkinfo_t *chkinfo;

        chkinfo = (void *)buf;
        for (i = from; i < to; i++) {
                fid2cid(&chkid, fileid, i);

                ret = md_chunk_getinfo(fileid, &chkid, chkinfo, NULL);
                if (ret) {
                        if (ret == ENOENT)
                                continue;
                        else {
                                DWARN("chkid "CHKID_FORMAT"@"CHKID_FORMAT" ret(%d)%s\n",
                                                CHKID_ARG(&chkid), CHKID_ARG(fileid), ret, strerror(ret));
                                fail++;
                        }
                }

                ret = __recovery_chunk(nid, fileid, chkinfo);
                if (ret) {
                        DWARN("chkid "CHKID_FORMAT" ret(%d)%s\n", CHKID_ARG(fileid), ret, strerror(ret));
                        fail++;
                }
        }

        fid2cid(&chkid, fileid, from);
        cid2tid(&_from, &chkid);
        fid2cid(&chkid, fileid, to);
        cid2tid(&_to, &chkid);

        for (i = _from.idx; i <= (int)_to.idx; i++) {
                chkid = _from;
                chkid.idx = i;

                //DINFO("chunk "CHKID_FORMAT"\n", CHKID_ARG(&chkid));
                ret = md_chunk_getinfo(fileid, &chkid, chkinfo, NULL);
                if (ret) {
                        if (ret == ENOENT)
                                continue;
                        else {
                                DWARN("chkid "CHKID_FORMAT"@"CHKID_FORMAT" ret(%d)%s\n",
                                                CHKID_ARG(&chkid), CHKID_ARG(fileid), ret, strerror(ret));
                                fail++;
                        }
                }

                ret = __recovery_chunk(nid, fileid, chkinfo);
                if (ret) {
                        DWARN("chkid "CHKID_FORMAT" ret(%d)%s\n", CHKID_ARG(fileid), ret, strerror(ret));
                        fail++;
                }
        }

        *_fail = fail;
}

typedef struct {
        sem_t sem;
        int from;
        int to;
        nid_t nid;
        fileid_t fileid;
        int fail;
} args_t;

void *__recovery_file_range(void *_args)
{
        args_t *args;

        args = _args;

        __recovery_file_range__(&args->nid, &args->fileid, args->from, args->to, &args->fail);

        sem_post(&args->sem);

        return NULL;
}

static int __recovery_file__(const nid_t *nid, const fileid_t *fileid)
{
        int ret, retry = 0, thread, i, step = 0, err = 0;
        fileinfo_t fileinfo;
        args_t _args[100], *args;
        uint64_t chknum;
        pthread_t th;
        pthread_attr_t ta;

retry:
        ret = md_getattr(fileid, &fileinfo);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        chknum = size2chknum(fileinfo.size);

        if (chknum < 100)
                thread = 1;
        else if (chknum < 1000)
                thread = 10;
        else
                thread = 100;

        (void) pthread_attr_init(&ta);
        (void) pthread_attr_setdetachstate(&ta,PTHREAD_CREATE_DETACHED);

        step = chknum / thread;
        for (i = 0; i < thread; i++) {
                args = &_args[i];
                ret = sem_init(&args->sem, 0, 0);
                if (ret)
                        GOTO(err_ret, ret);

                args->fail = 0;
                args->nid = *nid;
                args->fileid = *fileid;
                args->from = i * step;
                args->to = (i + 1) * step;

                ret = pthread_create(&th, &ta, __recovery_file_range, args);
                if (ret)
                        GOTO(err_ret, ret);
        }

        for (i = 0; i < thread; i++) {
                args = &_args[i];
                ret = sem_wait(&args->sem);
                if (ret)
                        err = ret;

                if (args->fail) {
                        __fail__++;
                }
        }

        if (err) {
                ret = err;
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __recovery_file(const nid_t *nid, const fileid_t *fileid)
{
        int ret, delen, retry = 0;
        char buf[MAX_BUF_LEN], de0[BIG_BUF_LEN];
        chkinfo_t *chkinfo;
        struct dirent *de;
        uint64_t offset;

        ret = __recovery_file__(nid, fileid);
        if (ret)
                GOTO(err_ret, ret);

retry:
        delen = BIG_BUF_LEN;
        ret = stor_snapshot_list(fileid, de0, &delen);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        if (delen == 0)
                goto out;

        dir_for_each(de0, delen, de, offset) {
                if (strlen(de->d_name) == 0) {
                        break;
                }

                if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
                        continue;
                }

                if (!memcmp(de->d_name, LICH_SYSTEM_ATTR_UNLINK, strlen(LICH_SYSTEM_ATTR_UNLINK))) {
                        continue;
                }

                chkinfo = (void *)buf;
                ret = md_lookup_byname(fileid, de->d_name, chkinfo, NULL);
                if (ret)
                        GOTO(err_ret, ret);

                ret = __recovery_chunk(&chkinfo->diskid[0].id, &chkinfo->id, chkinfo);
                if (ret)
                        GOTO(err_ret, ret);

                ret = __recovery_file__(&chkinfo->diskid[0].id, &chkinfo->id);
                if (ret)
                        GOTO(err_ret, ret);
        }

out:
        return 0;
err_ret:
        return ret;
}

static int __recovery_dir__(const nid_t *nid, const fileid_t *fileid, const chkinfo_t *_chkinfo)
{
        int ret, retry;
        uint64_t i;
        chkid_t chkid;
        char buf[MAX_BUF_LEN];
        chkinfo_t *chkinfo;

        ret = __recovery_chunk(nid, fileid, _chkinfo);
        if (ret)
                GOTO(err_ret, ret);

        chkinfo = (void *)buf;
        i = 0;
        while (1) {
                chkid = *fileid;
                chkid.type = __POOL_SUB_CHUNK__;
                chkid.idx = i;

                retry = 0;
        retry:
                ret = md_chunk_getinfo(fileid, &chkid, chkinfo, NULL);
                if (ret) {
                        if (ret == ENOENT)
                                break;
                        else {
                                if (ret == EAGAIN) {
                                        SLEEP_RETRY(err_ret, ret, retry, retry);
                                } else
                                        GOTO(err_ret, ret);
                        }
                }

                ret = __recovery_chunk(nid, fileid, chkinfo);
                if (ret)
                        GOTO(err_ret, ret);

                i++;
        }

        return 0;
err_ret:
        return ret;
}

static int __recovery_dir(const nid_t *nid, const fileid_t *fileid)
{
        int ret, delen, retry = 0;
        struct dirent *de;
        char buf[MAX_BUF_LEN], de0[BIG_BUF_LEN];
        uint64_t offset;
        chkinfo_t *chkinfo;

        (void) nid;

        YASSERT(fileid->type == __POOL_CHUNK__);

        if (!chkid_cmp(&unlink_fid, fileid))
                goto out;

        offset = 0;
        while (1) {
        retry:
                delen = BIG_BUF_LEN;
                ret = md_listpool(fileid, offset, de0, &delen);
                if (ret) {
                        if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry, retry);
                        } else
                                GOTO(err_ret, ret);
                }

                if (delen == 0)
                        break;

                dir_for_each(de0, delen, de, offset) {
                        if (strcmp(de->d_name, ".") == 0
                            || strcmp(de->d_name, "..") == 0)
                                continue;

                        if (strlen(de->d_name) == 0)
                                goto out;

                        chkinfo = (void *)buf;
                        ret = md_lookup_byname(fileid, de->d_name, chkinfo, NULL);
                        if (ret)
                                GOTO(err_ret, ret);

                        //ret = md_chunk_check(&chkinfo->diskid[0].id, &chkinfo->id, &chkinfo->id, -1);
                        if (chkinfo->id.type == __VOLUME_CHUNK__) {
                                ret = __recovery_chunk(&chkinfo->diskid[0].id, &chkinfo->id, chkinfo);
                                if (ret)
                                        GOTO(err_ret, ret);

                                ret = __recovery_file(&chkinfo->diskid[0].id, &chkinfo->id);
                                if (ret)
                                        GOTO(err_ret, ret);
                        } else {
                                ret = __recovery_dir__(&chkinfo->diskid[0].id, &chkinfo->id, chkinfo);
                                if (ret)
                                        GOTO(err_ret, ret);

                                ret = __recovery_dir(&chkinfo->diskid[0].id, &chkinfo->id);
                                if (ret)
                                        GOTO(err_ret, ret);
                        }
                }
        }

out:
        return 0;
err_ret:
        return ret;
}

static int __mkunlink()
{
        int ret;
        fileid_t fileid, fileid1, fileid2;
        
        ret = stor_lookup1("/", &fileid);
        if (ret)
                GOTO(err_ret, ret);

        ret = stor_mkpool(&fileid, "system", NULL, &fileid1);
        if (ret) {
                if (ret == EEXIST) {
                        ret = stor_lookup1("/system", &fileid1);
                        if (ret)
                                GOTO(err_ret, ret);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_mkpool(&fileid1, "unlink", NULL, &fileid2);
        if (ret) {
                if (ret == EEXIST) {
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        if (ret == ENOSPC)
                ret = EAGAIN;
        return _errno(ret);
}

int recovery_bypath(const char *path, int deep)
{
        int ret;
        fileid_t fileid;
        chkinfo_t *chkinfo;
        char buf[MAX_BUF_LEN];

        __recovery__ = 1;
        __deep__ = deep;

        ret = stor_lookup1(path, &fileid);
        if (ret) {
                GOTO(err_ret, ret);
        }

retry:
        ret = stor_lookup1(UNLINK_ROOT, &unlink_fid);
        if (ret) {
                if (ret == ENOENT) {
                        ret = __mkunlink();
                        if (ret) {
                                GOTO(err_ret, ret);
                        }

                        goto retry;
                } else
                        GOTO(err_ret, ret);
        }

        if (!chkid_cmp(&unlink_fid, &fileid))
                goto out;

        chkinfo = (void *)buf;
        ret = md_chunk_getinfo(NULL, &fileid, chkinfo, NULL);
        if (ret)
                GOTO(err_ret, ret);

        ret = __recovery_chunk(&chkinfo->diskid[0].id, &chkinfo->id, chkinfo);
        if (ret)
                GOTO(err_ret, ret);

        if (fileid.type == __POOL_CHUNK__) {
                ret = __recovery_dir(&chkinfo->diskid[0].id, &chkinfo->id);
                if (ret)
                        GOTO(err_ret, ret);
        } else {
                ret = __recovery_file(&chkinfo->diskid[0].id, &chkinfo->id);
                if (ret)
                        GOTO(err_ret, ret);
        }

        if (__fail__ == 0) {
                printf("recovery complete successfully\n");
        } else {
                ret = EAGAIN;
                GOTO(err_ret, ret);
        }

out:
        return 0;
err_ret:
        printf("warning, recovery fail, left %u, please retry again\n", __fail__);
        return ret;
}

int recovery_scan(const char *path)
{
        int ret;
        fileid_t fileid;
        chkinfo_t *chkinfo;
        char buf[MAX_BUF_LEN];

        __recovery__ = 0;

        ret = stor_lookup1(path, &fileid);
        if (ret)
                GOTO(err_ret, ret);

retry:
        ret = stor_lookup1(UNLINK_ROOT, &unlink_fid);
        if (ret) {
                if (ret == ENOENT) {
                        ret = __mkunlink();
                        if (ret) {
                                GOTO(err_ret, ret);
                        }

                        goto retry;
                } else
                        GOTO(err_ret, ret);
        }

        if (!chkid_cmp(&unlink_fid, &fileid))
                goto out;

        chkinfo = (void *)buf;
        ret = md_chunk_getinfo(NULL, &fileid, chkinfo, NULL);
        if (ret)
                GOTO(err_ret, ret);

        ret = __recovery_chunk(&chkinfo->diskid[0].id, &chkinfo->id, chkinfo);
        if (ret)
                GOTO(err_ret, ret);

        if (fileid.type == __POOL_CHUNK__) {
                ret = __recovery_dir(&chkinfo->diskid[0].id, &chkinfo->id);
                if (ret)
                        GOTO(err_ret, ret);
        } else {
                ret = __recovery_file(&chkinfo->diskid[0].id, &chkinfo->id);
                if (ret)
                        GOTO(err_ret, ret);
        }

        if (__fail__ == 0) {
                printf("recovery complete successfully\n");
        } else {
                ret = EAGAIN;
                GOTO(err_ret, ret);
        }

out:
        return 0;
err_ret:
        printf("warning, recovery fail, left %u, please retry again\n", __fail__);
        return ret;
}

static int __disk_online(const chkinfo_t *chkinfo, int repnum, int *online)
{
        const nid_t *nid;

        nid = &chkinfo->diskid[repnum].id;

        if (net_islocal(nid)) {
                return replica_srv_diskonline(&chkinfo->id, online);
        } else {
                return replica_rpc_diskonline(nid, &chkinfo->id, online);
        }
}

int disk_online(const chkinfo_t *chkinfo, int repnum)
{
        int online;

        __disk_online(chkinfo, repnum, &online);

        return online;
}
