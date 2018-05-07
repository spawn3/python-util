#include "config.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <dirent.h>
#include <ctype.h>

#define DBG_SUBSYS S_LIBINTERFACE

#include "configure.h"
#include "env.h"
#include "adt.h"
#include "net_table.h"
#include "lichstor.h"
#include "cluster.h"
#include "metadata.h"
#include "storage.h"
#include "volume.h"
#include "../../storage/replica/replica.h"
#include "license.h"
#include "net_global.h"
#include "utils.h"
#include "dbg.h"
#include "lich_md.h"

#define MULTITHREADING 1

#define TIER_THREAD_MAX     10
#define TIER_SIZE_MIN       32      /* 32M */
#define TIER_TIER_MAX      3

typedef struct {
        fileid_t fileid;
        int tier;
        int verbose;
        void * (*worker)(void *_arg);
} arg_ext_t;

typedef struct {
        sem_t sem;
        int ret;
        arg_ext_t ext;
        off_t offset;
        size_t size;
} arg_t;

typedef struct {
        int count[TIER_TIER_MAX];
        uint64_t total;
        sy_spinlock_t lock;
} rate_t;

static rate_t *__rate__;

static int __lich_getid(const char *path, chkid_t *_chkid)
{
        int ret, retry = 0;
        const chkid_t *chkid;

        if (path[0] == '/') {
        retry:
                ret = stor_lookup1(path, _chkid);
                if (ret) {
                        if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry, retry);
                        } else
                                GOTO(err_ret, ret);
                }
        } else {
                chkid = str2id(path);
                if (chkid) {
                        *_chkid = *chkid;
                } else {
                        ret = EINVAL;
                        GOTO(err_ret, ret);
                }
        }

        return 0;
err_ret:
        return ret;
}

static int __tier_thread_create(arg_t * arg)
{
        int ret;
        pthread_t th;
        pthread_attr_t ta;

        ret = sem_init(&arg->sem, 0, 0);
        if (ret)
                GOTO(err_ret, ret);

        (void) pthread_attr_init(&ta);
        (void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);

        ret = pthread_create(&th, &ta, arg->ext.worker, arg);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int __tier_thread_startup(uint64_t chknum, arg_ext_t *arg_ext)
{
        int ret, count, i, err = 0;
        uint64_t left, offset, size, avg_size;
        arg_t args[TIER_THREAD_MAX];

        count = 0;
        offset = 0;
        left = chknum;
        avg_size = left % TIER_THREAD_MAX == 0 ? (left / TIER_THREAD_MAX) : (left / TIER_THREAD_MAX + 1);
        avg_size = avg_size < TIER_SIZE_MIN ? TIER_SIZE_MIN : avg_size;

        while (left > 0) {
                size = left < avg_size ? left : avg_size;
                args[count].offset = offset;
                args[count].size = size;
                args[count].ret = 0;
                args[count].ext = *arg_ext;

                ret = __tier_thread_create(&args[count]);
                if (ret)
                        GOTO(err_ret, ret);

                count ++;
                offset += size;
                left -= size;
        }

        for (i = 0; i < count; i++) {
                ret = sem_wait(&args[i].sem);
                if (ret)
                        err = ret;

                ret = args[i].ret;
                if (ret)
                        err = ret;
        }

        if (err) {
                ret = err;
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __tier_set_singlethreading(const char *name, int tier)
{
        int ret, j;
        fileid_t fileid, chkid;
        fileinfo_t fileinfo;
        chkinfo_t *chkinfo;
        char buf[MAX_BUF_LEN];
        uint64_t chknum, i;

        printf("set %s tier %u\n", name, tier);

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

#if 0
        if (fileid.type != __VOLUME_CHUNK__) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }
#endif
        
        ret = md_getattr(&fileid, &fileinfo);
        if (ret)
                GOTO(err_ret, ret);

        chkinfo = (void *)buf;
        chknum = size2chknum(fileinfo.size);
        for (i = 0; i < chknum; i++) {
                fid2cid(&chkid, &fileid, i);
                ret = md_chunk_getinfo(&fileid, &chkid, chkinfo, NULL);
                if (ret) {
                        if (ret == ENOENT) {
                                continue;
                        } else
                                GOTO(err_ret, ret);
                }

                for (j = 0; j < chkinfo->repnum; j++) {
                        ret = replica_rpc_settier(&chkinfo->diskid[j].id, &chkid, tier);
                        if (ret)
                                GOTO(err_ret, ret);

                        printf("set replica "CHKID_FORMAT" @ %s %u\n", CHKID_ARG(&chkid),
                               network_rname(&chkinfo->diskid[j].id), tier);
                }
        }        
        
        return 0;
err_ret:
        return ret;
}

static void * __tier_set_worker(void *_arg)
{
        int ret, i, j, max, retry = 0;
        arg_t *arg;
        fileid_t chkid;
        chkinfo_t *chkinfo;
        char buf[MAX_BUF_LEN];

        arg = _arg;
        max = arg->offset + arg->size;
        chkinfo = (void *)buf;

        for (i = arg->offset; i < max; i++) {
                fid2cid(&chkid, &arg->ext.fileid, i);
        retry1:
                ret = md_chunk_getinfo(&arg->ext.fileid, &chkid, chkinfo, NULL);
                if (ret) {
                        if (ret == ENOENT) {
                                continue;
                        } else if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry1, retry);
                        } else
                                GOTO(err_ret, ret);
                }

                for (j = 0; j < chkinfo->repnum; j++) {
                retry2:
                        ret = replica_rpc_settier(&chkinfo->diskid[j].id, &chkid, arg->ext.tier);
                        if (ret) {
                                if (ret == EAGAIN) {
                                        SLEEP_RETRY(err_ret, ret, retry2, retry);
                                } else
                                        GOTO(err_ret, ret);
                        }

                        ret = sy_spin_lock(&__rate__->lock);
                        if (ret)
                                GOTO(err_ret, ret);

                        __rate__->total++;

                        sy_spin_unlock(&__rate__->lock);

                        if (arg->ext.verbose)
                                printf("set replica "CHKID_FORMAT" @ %s %u\n", CHKID_ARG(&chkid),
                                       network_rname(&chkinfo->diskid[j].id), arg->ext.tier);
                }
        }

        arg->ret = 0;
        sem_post(&arg->sem);

        return NULL;
err_ret:
        arg->ret = ret;
        sem_post(&arg->sem);
        return NULL;
}

static int __tier_set_multithreading(const char *name, int tier, int verbose)
{
        int ret, retry = 0;
        fileid_t fileid;
        fileinfo_t fileinfo;
        uint64_t chknum;
        arg_ext_t arg_ext;

        printf("set %s tier %u\n", name, tier);

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

#if 0
        if (fileid.type != __VOLUME_CHUNK__) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }
#endif

retry:
        ret = md_getattr(&fileid, &fileinfo);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        chknum = size2chknum(fileinfo.size);
        arg_ext.fileid = fileid;
        arg_ext.tier = tier;
        arg_ext.verbose = verbose;
        arg_ext.worker = __tier_set_worker;

        ret = __tier_thread_startup(chknum, &arg_ext);
        if (ret)
                GOTO(err_ret, ret);

        printf("replica : %ld\n", chknum * fileinfo.repnum);
        printf("allocated : %ld\n", __rate__->total);

        return 0;
err_ret:
        return ret;
}

int lich_tier_set(const char *name, int tier, int verbose)
{
        int ret;

        if (tier != 0 && tier != 1) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        ret = ymalloc((void **)&__rate__, sizeof(*__rate__));
        if (ret)
                GOTO(err_ret, ret);

        memset(__rate__, 0x0, sizeof(*__rate__));

        ret = sy_spin_init(&__rate__->lock);
        if (ret)
                GOTO(err_free, ret);

        if (MULTITHREADING) {
                ret =__tier_set_multithreading(name, tier, verbose);
                if (ret)
                        GOTO(err_free, ret);
        } else {
                ret = __tier_set_singlethreading(name, tier);
                if (ret)
                        GOTO(err_free, ret);
        }

        yfree((void **)&__rate__);

        return 0;
err_free:
        yfree((void **)&__rate__);
err_ret:
        return ret;
}

int __tier_get_singlethreading(const char *name)
{
        int ret, j, tier;
        fileid_t fileid, chkid;
        fileinfo_t fileinfo;
        chkinfo_t *chkinfo;
        char buf[MAX_BUF_LEN];
        uint64_t chknum, i;

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

#if 0
        if (fileid.type != __VOLUME_CHUNK__) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }
#endif

        ret = md_getattr(&fileid, &fileinfo);
        if (ret)
                GOTO(err_ret, ret);

        chkinfo = (void *)buf;
        chknum = size2chknum(fileinfo.size);
        for (i = 0; i < chknum; i++) {
                fid2cid(&chkid, &fileid, i);
                ret = md_chunk_getinfo(&fileid, &chkid, chkinfo, NULL);
                if (ret) {
                        if (ret == ENOENT) {
                                continue;
                        } else
                                GOTO(err_ret, ret);
                }

                for (j = 0; j < chkinfo->repnum; j++) {
                        ret = replica_rpc_gettier(&chkinfo->diskid[j].id, &chkid, &tier);
                        if (ret)
                                GOTO(err_ret, ret);

                        printf("get replica "CHKID_FORMAT" @ %s %u\n", CHKID_ARG(&chkid),
                               network_rname(&chkinfo->diskid[j].id), tier);
                }
        }

        return 0;
err_ret:
        return ret;
}

static void * __tier_get_worker(void *_arg)
{
        int ret, i, j, max, tier, retry = 0;
        arg_t *arg;
        fileid_t chkid;
        chkinfo_t *chkinfo;
        char buf[MAX_BUF_LEN];

        arg = _arg;
        max = arg->offset + arg->size;
        chkinfo = (void *)buf;

        for (i = arg->offset; i < max; i++) {
                fid2cid(&chkid, &arg->ext.fileid, i);
        retry1:
                ret = md_chunk_getinfo(&arg->ext.fileid, &chkid, chkinfo, NULL);
                if (ret) {
                        if (ret == ENOENT) {
                                continue;
                        } else if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry1, retry);
                        } else
                                GOTO(err_ret, ret);
                }

                for (j = 0; j < chkinfo->repnum; j++) {
                retry2:
                        ret = replica_rpc_gettier(&chkinfo->diskid[j].id, &chkid, &tier);
                        if (ret) {
                                if (ret == EAGAIN) {
                                        SLEEP_RETRY(err_ret, ret, retry2, retry);
                                } else
                                        GOTO(err_ret, ret);
                        }

                        ret = sy_spin_lock(&__rate__->lock);
                        if (ret)
                                GOTO(err_ret, ret);

                        __rate__->total++;
                        __rate__->count[tier]++;

                        ret = sy_spin_unlock(&__rate__->lock);
                        if (ret)
                                GOTO(err_ret, ret);

                        if (arg->ext.verbose)
                                printf("get replica "CHKID_FORMAT" @ %s %u\n", CHKID_ARG(&chkid),
                                       network_rname(&chkinfo->diskid[j].id), tier);
                }
        }

        arg->ret = 0;
        sem_post(&arg->sem);

        return NULL;
err_ret:
        arg->ret = ret;
        sem_post(&arg->sem);
        return NULL;
}

int __tier_get_multithreading(const char *name, int verbose)
{
        int ret, retry = 0;
        fileid_t fileid;
        fileinfo_t fileinfo;
        uint64_t chknum;
        arg_ext_t arg_ext;

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

#if 0
        if (fileid.type != __VOLUME_CHUNK__) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }
#endif

retry:
        ret = md_getattr(&fileid, &fileinfo);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        chknum = size2chknum(fileinfo.size);
        arg_ext.fileid = fileid;
        arg_ext.verbose = verbose;
        arg_ext.worker = __tier_get_worker;

        ret = __tier_thread_startup(chknum, &arg_ext);
        if (ret)
                GOTO(err_ret, ret);

        printf("replica : %ld\n", chknum * fileinfo.repnum);
        printf("allocated : %ld\n", __rate__->total);
        printf("tier0 : %d\n", __rate__->count[0]);
        printf("tier1 : %d\n", __rate__->count[1]);

        return 0;
err_ret:
        return ret;
}


int lich_tier_get(const char *name, int verbose)
{
        int ret;

        ret = ymalloc((void **)&__rate__, sizeof(*__rate__));
        if (ret)
                GOTO(err_ret, ret);

        memset(__rate__, 0x0, sizeof(*__rate__));

        ret = sy_spin_init(&__rate__->lock);
        if (ret)
                GOTO(err_free, ret);

        if (MULTITHREADING) {
                ret =__tier_get_multithreading(name, verbose);
                if (ret)
                        GOTO(err_free, ret);
        } else {
                ret = __tier_get_singlethreading(name);
                if (ret)
                        GOTO(err_free, ret);
        }

        yfree((void **)&__rate__);

        return 0;
err_free:
        yfree((void **)&__rate__);
err_ret:
        return ret;
}

static void * __priority_set_worker(void *_arg)
{
        int ret, i, j, max, retry = 0;
        arg_t *arg;
        fileid_t chkid;
        chkinfo_t *chkinfo;
        char buf[MAX_BUF_LEN];

        arg = _arg;
        max = arg->offset + arg->size;
        chkinfo = (void *)buf;

        for (i = arg->offset; i < max; i++) {
                fid2cid(&chkid, &arg->ext.fileid, i);
        retry1:
                ret = md_chunk_getinfo(&arg->ext.fileid, &chkid, chkinfo, NULL);
                if (ret) {
                        if (ret == ENOENT) {
                                continue;
                        } else if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry1, retry);
                        } else
                                GOTO(err_ret, ret);
                }

                for (j = 0; j < chkinfo->repnum; j++) {
                retry2:
                        ret = replica_rpc_setpriority(&chkinfo->diskid[j].id, &chkid, arg->ext.tier);
                        if (ret) {
                                if (ret == EAGAIN) {
                                        SLEEP_RETRY(err_ret, ret, retry2, retry);
                                } else
                                        GOTO(err_ret, ret);
                        }

                        ret = sy_spin_lock(&__rate__->lock);
                        if (ret)
                                GOTO(err_ret, ret);

                        __rate__->total++;

                        sy_spin_unlock(&__rate__->lock);

                        if (arg->ext.verbose && arg->ext.tier == -1)
                                printf("set replica "CHKID_FORMAT" @ %s default\n", CHKID_ARG(&chkid),
                                       network_rname(&chkinfo->diskid[j].id));
                        else if (arg->ext.verbose)
                                printf("set replica "CHKID_FORMAT" @ %s %u\n", CHKID_ARG(&chkid),
                                       network_rname(&chkinfo->diskid[j].id), arg->ext.tier);
                }
        }

        arg->ret = 0;
        sem_post(&arg->sem);

        return NULL;
err_ret:
        arg->ret = ret;
        sem_post(&arg->sem);
        return NULL;
}

static int __priority_set_multithreading(const char *name, int priority, int verbose)
{
        int ret, retry = 0;
        fileid_t fileid;
        fileinfo_t fileinfo;
        uint64_t chknum;
        arg_ext_t arg_ext;
        setattr_t setattr;

        if (priority == -1)
                printf("set %s priority default\n", name);
        else
                printf("set %s priority %u\n", name, priority);

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

#if 0
        if (fileid.type != __VOLUME_CHUNK__) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }
#endif

retry:
        ret = md_getattr(&fileid, &fileinfo);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        if ((int)fileinfo.priority != priority) {
                memset(&setattr, 0x0, sizeof(setattr));
                setattr.priority.set_it = 1;
                setattr.priority.priority = priority;
                ret = md_setattr(&fileid, NULL, &setattr);
                if (ret) {
                        GOTO(err_ret, ret);
                }
        }

        chknum = size2chknum(fileinfo.size);
        arg_ext.fileid = fileid;
        arg_ext.tier = priority;
        arg_ext.verbose = verbose;
        arg_ext.worker = __priority_set_worker;

        ret = __tier_thread_startup(chknum, &arg_ext);
        if (ret)
                GOTO(err_ret, ret);

        printf("replica : %ld\n", chknum * fileinfo.repnum);
        printf("allocated : %ld\n", __rate__->total);

        return 0;
err_ret:
        return ret;
}

static int __priority_set_pithy(const char *name, int priority)
{
        int ret, retry = 0;
        fileid_t fileid;
        fileinfo_t fileinfo;
        setattr_t setattr;

        if (priority == -1)
                printf("set %s priority default\n", name);
        else
                printf("set %s priority %u\n", name, priority);

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

retry:
        ret = md_getattr(&fileid, &fileinfo);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        if ((int)fileinfo.priority != priority) {
                memset(&setattr, 0x0, sizeof(setattr));
                setattr.priority.set_it = 1;
                setattr.priority.priority = priority;
                ret = md_setattr(&fileid, NULL, &setattr);
                if (ret) {
                        GOTO(err_ret, ret);
                }
        }

        return 0;
err_ret:
        return ret;
}

int lich_priority_set(const char *name, int priority, int verbose, int pithy)
{
        int ret;

        if (priority != 0 && priority != 1 && priority != -1) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        ret = ymalloc((void **)&__rate__, sizeof(*__rate__));
        if (ret)
                GOTO(err_ret, ret);

        memset(__rate__, 0x0, sizeof(*__rate__));

        ret = sy_spin_init(&__rate__->lock);
        if (ret)
                GOTO(err_free, ret);

        if (pithy) {
                ret =__priority_set_pithy(name, priority);
                if (ret)
                        GOTO(err_free, ret);
        } else {
                ret =__priority_set_multithreading(name, priority, verbose);
                if (ret)
                        GOTO(err_free, ret);
        }

        yfree((void **)&__rate__);

        return 0;
err_free:
        yfree((void **)&__rate__);
err_ret:
        return ret;
}

static void * __priority_get_worker(void *_arg)
{
        int ret, i, j, max, priority, retry = 0;
        arg_t *arg;
        fileid_t chkid;
        chkinfo_t *chkinfo;
        char buf[MAX_BUF_LEN];

        arg = _arg;
        max = arg->offset + arg->size;
        chkinfo = (void *)buf;

        for (i = arg->offset; i < max; i++) {
                fid2cid(&chkid, &arg->ext.fileid, i);
        retry1:
                ret = md_chunk_getinfo(&arg->ext.fileid, &chkid, chkinfo, NULL);
                if (ret) {
                        if (ret == ENOENT) {
                                continue;
                        } else if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry1, retry);
                        } else
                                GOTO(err_ret, ret);
                }

                for (j = 0; j < chkinfo->repnum; j++) {
                retry2:
                        ret = replica_rpc_getpriority(&chkinfo->diskid[j].id, &chkid, &priority);
                        if (ret) {
                                if (ret == EAGAIN) {
                                        SLEEP_RETRY(err_ret, ret, retry2, retry);
                                } else
                                        GOTO(err_ret, ret);
                        }

                        ret = sy_spin_lock(&__rate__->lock);
                        if (ret)
                                GOTO(err_ret, ret);

                        __rate__->total++;
                        __rate__->count[priority + 1]++;

                        ret = sy_spin_unlock(&__rate__->lock);
                        if (ret)
                                GOTO(err_ret, ret);

                        if (arg->ext.verbose && priority == -1)
                                printf("get replica "CHKID_FORMAT" @ %s default\n", CHKID_ARG(&chkid),
                                       network_rname(&chkinfo->diskid[j].id));
                        else if (arg->ext.verbose)
                                printf("get replica "CHKID_FORMAT" @ %s %u\n", CHKID_ARG(&chkid),
                                       network_rname(&chkinfo->diskid[j].id), priority);
                }
        }

        arg->ret = 0;
        sem_post(&arg->sem);

        return NULL;
err_ret:
        arg->ret = ret;
        sem_post(&arg->sem);
        return NULL;
}

static int __priority_get_multithreading(const char *name, int verbose)
{
        int ret, retry = 0;
        fileid_t fileid;
        fileinfo_t fileinfo;
        uint64_t chknum;
        arg_ext_t arg_ext;

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

#if 0
        if (fileid.type != __VOLUME_CHUNK__) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }
#endif

retry:
        ret = md_getattr(&fileid, &fileinfo);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        chknum = size2chknum(fileinfo.size);
        arg_ext.fileid = fileid;
        arg_ext.verbose = verbose;
        arg_ext.worker = __priority_get_worker;

        ret = __tier_thread_startup(chknum, &arg_ext);
        if (ret)
                GOTO(err_ret, ret);

        if ((int)fileinfo.priority == -1)
                printf("priority : default\n");
        else
                printf("priority : %d\n", fileinfo.priority);
        printf("replica : %ld\n", chknum * fileinfo.repnum);
        printf("allocated : %ld\n", __rate__->total);
        printf("default : %d\n", __rate__->count[0]);
        printf("priority0 : %d\n", __rate__->count[1]);
        printf("priority1 : %d\n", __rate__->count[2]);

        return 0;
err_ret:
        return ret;
}

static int __priority_get_pithy(const char *name)
{
        int ret, retry = 0;
        fileid_t fileid;
        fileinfo_t fileinfo;

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

retry:
        ret = md_getattr(&fileid, &fileinfo);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        printf("chkid : "CHKID_FORMAT"\n", CHKID_ARG(&fileid));
        if ((int)fileinfo.priority == -1)
                printf("priority : default\n");
        else
                printf("priority : %d\n", fileinfo.priority);

        return 0;
err_ret:
        return ret;
}

int lich_priority_get(const char *name, int verbose, int pithy)
{
        int ret;

        ret = ymalloc((void **)&__rate__, sizeof(*__rate__));
        if (ret)
                GOTO(err_ret, ret);

        memset(__rate__, 0x0, sizeof(*__rate__));

        ret = sy_spin_init(&__rate__->lock);
        if (ret)
                GOTO(err_free, ret);

        if (pithy) {
                ret =__priority_get_pithy(name);
                if (ret)
                        GOTO(err_free, ret);
        } else {
                ret =__priority_get_multithreading(name, verbose);
                if (ret)
                        GOTO(err_free, ret);
        }

        yfree((void **)&__rate__);

        return 0;
err_free:
        yfree((void **)&__rate__);
err_ret:
        return ret;
}

int __localize_get(const char *name, int *localize)
{
        int ret, retry;
        fileid_t fileid;
        fileinfo_t fileinfo;

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        retry = 0;
retry:
        ret = md_getattr(&fileid, &fileinfo);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        if ((fileinfo.attr & __FILE_ATTR_LOCALIZE__) ==  __FILE_ATTR_LOCALIZE__) {
                *localize = 1;
        } else {
                *localize = 0;
        }


        return 0;
err_ret:
        return ret;
}

int lich_localize_get(const char *name)
{
        int ret, num;

        ret = __localize_get(name, &num);
        if (ret)
                GOTO(err_ret, ret);

        printf("path: %s, localize: %d\n", name, num);
        return 0;
err_ret:
        return ret;
}

int lich_localize_set(const char *name, int num)
{
        int ret, localize;
        fileid_t fileid;
        setattr_t setattr;

        ret = __localize_get(name, &localize);
        if (ret)
                GOTO(err_ret, ret);

        if (localize == num) {
                goto out;
        }

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        memset(&setattr, 0x0, sizeof(setattr));
        setattr.localize.set_it = 1;
        setattr.localize.localize = num;
        ret = md_setattr(&fileid, NULL, &setattr);
        if (ret) {
                GOTO(err_ret, ret);
        }

out:
        return 0;
err_ret:
        return ret;
}

int __multpath_get(const char *name, int *multpath)
{
        int ret, retry;
        fileid_t fileid;
        fileinfo_t fileinfo;

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

#if 0
        if (fileid.type != __VOLUME_CHUNK__) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }
#endif

        retry = 0;
retry:
        ret = md_getattr(&fileid, &fileinfo);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        if ((fileinfo.attr & __FILE_ATTR_MULTPATH__) ==  __FILE_ATTR_MULTPATH__) {
                *multpath = 1;
        } else {
                *multpath = 0;
        }


        return 0;
err_ret:
        return ret;
}

int lich_multpath_get(const char *name)
{
        int ret, num;
        ret = __multpath_get(name, &num);
        if (ret)
                GOTO(err_ret, ret);

        printf("path: %s, multpath: %d\n", name, num);
        return 0;
err_ret:
        return ret;

}

int lich_multpath_set(const char *name, int num)
{
        int ret, multpath;
        fileid_t fileid;
        setattr_t setattr;

        ret = __multpath_get(name, &multpath);
        if (ret)
                GOTO(err_ret, ret);

        if (multpath == num) {
                goto out;
        }

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

#if 0
        if (fileid.type != __VOLUME_CHUNK__) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }
#endif

        memset(&setattr, 0x0, sizeof(setattr));
        setattr.multpath.set_it = 1;
        setattr.multpath.multpath = num;
        ret = md_setattr(&fileid, NULL, &setattr);
        if (ret) {
                GOTO(err_ret, ret);
        }

out:
        return 0;
err_ret:
        return ret;
}
