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
#include "utils.h"
#include "configure.h"
#include "dbg.h"

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

static int __scan(const char *_path, int _recover, int _full)
{
        int ret, recover_succ;
        char path[MAX_BUF_LEN];
        msgqueue_t lost, recover;
        uint64_t lost_count, recover_count;

        (void) _full;

        snprintf(path, MAX_PATH_LEN, "/dev/shm/lich4/scan/lost");
        ret = msgqueue_init(&lost, path, MSGQUEUE_SIZE, 0);
        if (ret) {
                if (ret == EBUSY) {
                        printf("another scan running\n");
                        EXIT(EBUSY);
                } else
                        GOTO(err_ret, ret);
        }

        snprintf(path, MAX_PATH_LEN, "/dev/shm/lich4/scan/recover");
        ret = msgqueue_init(&recover, path, MSGQUEUE_SIZE, 0);
        if (ret) {
                if (ret == EBUSY) {
                        printf("another scan running\n");
                        EXIT(EBUSY);
                } else
                        GOTO(err_ret, ret);
        }

        ret = iterate_all(_path, __verbose, &recover, &recover_count, &lost, &lost_count, _full);
        if (ret)
                GOTO(err_queue, ret);

        if (recover_count)
                scan.needretry = 1;

        printf("\n");

        if (_recover) {
                ret = recover_queue(&recover, recover_count, scan.parallels, &scan.needretry, &recover_succ);
                if (ret)
                        GOTO(err_queue, ret);

                if (scan.needretry == 0 && !msgqueue_empty(&lost)) {
                        printf("    lost chunk %llu, node list:\n", (LLU)lost_count);
                        __red__ = 1;
                        __scan_analysis(&lost, SYNC_STAT_LOST);
                        __red__ = 0;
                }
        } else {
                if (!msgqueue_empty(&recover)) {
                        printf("    recoverable chunk %llu, node list:\n", (LLU)recover_count);
                        __scan_analysis(&recover, SYNC_STAT_DIRTY);
                }

                if (!msgqueue_empty(&lost)) {
                        printf("    lost chunk %llu, node list:\n", (LLU)lost_count);
                        __red__ = 1;
                        __scan_analysis(&lost, SYNC_STAT_LOST);
                        __red__ = 0;
                }
        }

        msgqueue_close(&recover);
        rmdir(recover.home);
        msgqueue_close(&lost);
        rmdir(lost.home);

        return 0;
err_queue:
        msgqueue_close(&recover);
        rmdir(recover.home);
        msgqueue_close(&lost);
        rmdir(lost.home);
err_ret:
        return ret;
}

int lich_scan(const char *path, int verbose, int recover, int full)
{
        int ret, step, retry1 = 0;

        __verbose = verbose;
        //if (recover == 0)
        //__verbose = 1;

        step = 0;
        scan.parallels = 100;
        while (1) {
                scan.needretry = 0;

        retry1:

                if (__verbose) {
                        printf("=== scan time %u ===\n", step);
                        printf("    scan ...\n");
                }

                if (step < 1) {
                        scan.needretry = 1;
                }

                ret = __scan(path, recover, full);
                if (ret) {
                        DWARN("scan meta fail, retry in 1\n");
                        scan.needretry = 1;
                        SLEEP_RETRY(err_ret, ret, retry1, retry1);
                }

                if (recover == 0)
                        break;

                step++;

                if (scan.needretry) {
                        continue;
                } 

                break;
        }

        //printf("scan %s finsh time %u\n", path, step);

        return 0;
err_ret:
        return object_errno(ret);
}
