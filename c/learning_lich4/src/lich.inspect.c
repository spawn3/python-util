
#include "config.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <dirent.h>
#include <ctype.h>
#include <regex.h>

#define DBG_SUBSYS S_LIBINTERFACE

#include "configure.h"
#include "env.h"
#include "adt.h"
#include "net_table.h"
#include "cluster.h"
#include "fileinfo.h"
#include "metadata.h"
#include "lich_md.h"
#include "chk_proto.h"
#include "../../storage/chunk/chunk_proto.h"
#include "../../storage/replica/replica.h"
#include "../../storage/storage/md_parent.h"
#include "../../storage/storage/md_root.h"
#include "../../storage/storage/stor_rpc.h"
#include "lichstor.h"
#include "license.h"
#include "utils.h"
#include "net_global.h"
#include "dbg.h"
#include "chunk.h"
#include <sqlite3.h>


#define ALLOCATE_THREAD_MAX 10

static int __lookup_find = 0;
static fileid_t unlink_fid;    //skip /system/unlink directory

typedef enum {
        OP_NULL,
        OP_LIST,
        OP_STAT,
        OP_LOOKUP,
        OP_CHUNKSET,
        OP_CMP,
        OP_MOVE,
        OP_CHUNK_INFO,
        OP_CONNECTION,
        OP_SCAN,
        OP_RECOVER,
        OP_RECOVERCHUNK,
        OP_PAXOSDUMP,
        OP_BLANCEOBJ,
        OP_ALLOCATE,
        OP_MD5SUM,
        OP_TIER,
        OP_PRIORITY,
        OP_CHUNK_MOVE,
        OP_CHUNK_STAT,
        OP_DB_DUMP,
        OP_BASE64,
        OP_MULTPATH,
        OP_LOCALIZE,
        OP_CHUNK_DUMP,
        OP_CHUNK_FILL,
} admin_op_t;

typedef struct {
        fileid_t fileid;
        int ret;
        off_t begin;
        off_t end;
} allocate_arg_t;

static void usage()
{
        fprintf(stderr, "\nusage:\n"
                "lich.inspect --list  <chkid/path>\n"
                "lich.inspect --stat <chkid/path>\n"
                "lich.inspect --lookup <chkid/path>\n"
                "lich.inspect --allocate  <chkid/path> [--single]\n"
                "lich.inspect --move, -m  <chkid/path> <nodename> [--async]\n"
                "lich.inspect --recover <path> [--deep]\n"
                "lich.inspect --scan <path>\n"
                "\n"
                "lich.inspect --tier <path> [0|1]\n"
                "lich.inspect --priority <path> [default | 0|1]\n"
                "lich.inspect --localize <path> [0|1]\n"
                "lich.inspect --multpath <path> [0|1] \n"
                "\n"
                "lich.inspect --chunkset  <chkid/path> <nodename> <clean | dirty | check>\n"
                "lich.inspect --chunkmove <chkid> <nodename,nodename,...>\n"
                "lich.inspect --chunkstat <chkid>\n"
                "lich.inspect --chunkinfo <chkid/path> [-p|--pithy] [-v]\n"
                "lich.inspect --chunkdump <chkid/path> [dst]\n"
                "lich.inspect --chunkfill <chkid/path> [src]\n"
                "\n"
                "lich.inspect --paxosdump\n"
                "lich.inspect --md5sum <path>\n"
                "lich.inspect --dbdump <idx/all>\n"
                "lich.inspect --base64 <encode/decode> <chkid/base64code> [-p|--pithy]\n"
                "\nexplain:\n"
                "        get the value of tier/priority/localize/multpath if omit [arg],\n"
                "        else it was set the value\n"
                );
}

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

#if 0
static int __lich_config_dump()
{
        int ret;
        int parallel = 0, expdata = 0;
        int parallel_recover = 0, parallel_balance = 0;
        char hostname[MAX_NAME_LEN];

        ret = conf_init();
        if (ret)
                GOTO(err_ret, ret);

        dbg_sub_init();

        ret = net_gethostname(hostname, MAX_NAME_LEN);
        if (ret) {
                if (ret == ECONNREFUSED)
                        strcpy(hostname, "N/A");
                else
                        GOTO(err_ret, ret);
        }

        ret = balance_parallel_get(&parallel, &expdata, &parallel_balance);
        if (ret)
                GOTO(err_ret, ret);

        ret = recover_parallel_get(&parallel, &expdata, &parallel_recover);
        if (ret)
                GOTO(err_ret, ret);

        printf("globals.clustername:%s\n"
               "globals.hostname:%s\n"
               "globals.home:%s\n"
               "globals.localize:%d\n"
               "globals.wmem_max:%llu\n"
               "globals.rmem_max:%llu\n"
               "globals.replica_max:%u\n"
               "globals.crontab:%u\n"
               "globals.nohosts:%s\n"
               "globals.cgroup:%s\n"
               "globals.testing:%s\n"
               "metadata.meta:%d\n"
               "iscsi.iqn:%s\n"
               "iscsi.port:%d\n"
               "parallel_recover:%d\n"
               "parallel_balance:%d\n",
               gloconf.cluster_name,
               hostname,
               gloconf.home,
               gloconf.localize,
               (LLU)gloconf.wmem_max,
               (LLU)gloconf.rmem_max,
               LICH_REPLICA_MAX,
               gloconf.crontab,
               gloconf.nohosts ? "on" : "off",
               gloconf.cgroup ? "on" : "off",
               gloconf.testing ? "on" : "off",
               mdsconf.meta,
               sanconf.iqn,
               sanconf.iscsi_port,
               parallel_recover,
               parallel_balance);

        return 0;
err_ret:
        return ret;
}
#endif

static int __inspect_init(int op)
{
        int ret;

        ret = env_init_simple("lich.inspect");
        if (ret)
                GOTO(err_ret, ret);

        ret = network_connect_master();
        if (ret) {
                GOTO(err_ret, ret);
        }

        ret = stor_init(NULL, -1);
        if (ret)
                GOTO(err_ret, ret);

        if (op == OP_MOVE || op == OP_RECOVER || op == OP_SCAN || op == OP_ALLOCATE) {
                ret = storage_license_check();
                if (ret)
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __lich_list(const char *path)
{
        int ret, delen;
        fileid_t fileid;
        uint64_t offset;
        struct dirent *de;
        void *de0;

        YASSERT(path);
        YASSERT(strlen(path));

        ret = stor_lookup1(path, &fileid);
        if (ret) {
                GOTO(err_ret, ret);
        }

        offset = 0;
        while (1) {
                delen = MAX_BUF_LEN;
                ret = stor_listpool(&fileid, offset, &de0, &delen);
                if (ret)
                        GOTO(err_ret, ret);

                if (delen == 0)
                        break;

                dir_for_each(de0, delen, de, offset) {
                        if (strlen(de->d_name) == 0)
                                goto out;

                        printf("%s\n", de->d_name);
                }

                yfree((void **)&de0);
        }

out:
        return 0;
err_ret:
        return ret;
}

static int __lich_dump(const char *path, int verbose, int pithy)
{
        int ret;
        chkid_t id;
        char buf[MAX_BUF_LEN];

        verbose = 1;

        ret = __lich_getid(path, &id);
        if (ret) {
                GOTO(err_print, ret);
        }

        ret = stor_stat(&id, buf, verbose);
        if (ret) {
                GOTO(err_print, ret);
        }

        printf("%s", buf);

        if (verbose && !pithy && id.type == __VOLUME_CHUNK__) {
                ret = stor_stat_file(&id);
                if (ret) {
                        GOTO(err_ret, ret);
                }
        }

        return 0;
err_print:
        if (ret == EAGAIN)
                printf("\x1b[1;31m  *%s : offline\x1b[0m\n", path);
err_ret:
        return ret;
}

static int __lich_cmp_dump(const chkid_t *chkid, char *_buf)
{
        int ret, i, err;
        sha1_result_t *result;
        char buf[MAX_BUF_LEN], tmp[MAX_BUF_LEN];
        chkinfo_t *chkinfo;

        chkinfo = (void *)tmp;
        ret = md_chunk_getinfo(NULL, chkid, chkinfo, NULL);
        if (ret)
                GOTO(err_ret, ret);

        result = (void *)buf;
        ret = chunk_proto_sha1(chkinfo, result);
        if (ret)
                GOTO(err_ret, ret);

        for (err = 0, i = 1; i < (int)result->count; i++) {
                if (strcmp(result->md[0], result->md[i]) != 0) {
                        snprintf(_buf, MAX_BUF_LEN, "\x1b[1;31m"CHKID_FORMAT" %s --> %s error\x1b[0m",
                                 CHKID_ARG(chkid), result->md[0], result->md[i]);
                        err++;
                }
        }

        if (err == 0) {
                snprintf(_buf, MAX_BUF_LEN, ""CHKID_FORMAT" sha1 %s",
                         CHKID_ARG(chkid), result->md[0]);
        }

        return 0;
err_ret:
        return ret;
}

static int __lich_cmp(const char *path)
{
        int ret;
        fileid_t fileid;
        char tmp[MAX_BUF_LEN];
        chkid_t chkid;
        uint64_t i, chknum;
        fileinfo_t fileinfo;
        int retry;

        ret = __lich_getid(path, &fileid);
        if (ret) {
                GOTO(err_ret, ret);
        }

        ret = __lich_cmp_dump(&fileid, tmp);
        if (ret)
                GOTO(err_ret, ret);

        printf("check %s:\n", tmp);

        if (fileid.type == __VOLUME_CHUNK__) {
                retry = 0;
retry:
                ret = md_getattr(&fileid, &fileinfo);
                if (ret) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                }

                chknum = size2chknum(fileinfo.size);

                printf("chunknum: %lu\n", chknum);
                for (i = 0; i < chknum; i++) {
                        fid2cid(&chkid, &fileid, i);

                        retry = 0;
retry2:
                        ret = __lich_cmp_dump(&chkid, tmp);
                        if (ret) {
                                if (ret == ENOENT) {
                                        continue;
                                } else {
                                        SLEEP_RETRY(err_ret, ret, retry2, retry);
                                }
                        }

                        printf("check "CHKID_FORMAT"[%llu]: %s\n", CHKID_ARG(&fileid), (LLU)i, tmp);
                }
        }

        return 0;
err_ret:
        return ret;
}

static int __str2stat(const char *stat, int *_s)
{
        int ret, s;
        if (strcmp(stat, "clean") == 0) {
                s = __S_CLEAN;
        } else if (strcmp(stat, "check") == 0) {
                s = __S_CHECK;
        } else if (strcmp(stat, "dirty") == 0) {
                s = __S_DIRTY;
        } else {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        *_s = s;

        return 0;
err_ret:
        return ret;
}

static int __lich_chunkset(const char *path, const char *node, const char *_status)
{
        int ret, status;
        chkid_t chkid;
        nid_t nid;
        char buf[MAX_BUF_LEN];

        printf("set chunk %s @ %s status %s\n", path, node, _status);

retry:
        printf("That dangerous, are you sure what are you doing? (Yes/No)");

        ret = scanf("%s", buf);
        if (ret != 1) {
                ret = EINVAL;
                goto retry;
        }

        if (strcasecmp(buf, "No") == 0) {
                ret = EPERM;
                GOTO(err_ret, ret);
        } else if (strcasecmp(buf, "Yes") == 0) {
                //pass
        } else {
                goto retry;
        }

        ret = __lich_getid(path, &chkid);
        if (ret)
                GOTO(err_ret, ret);

        ret = network_connect_byname(node, &nid);
        if (ret)
                GOTO(err_ret, ret);

        ret = __str2stat(_status, &status);
        if (ret)
                GOTO(err_ret, ret);

#if 0
        ret = md_parent_get(&chkid, &parent);
        if (ret)
                GOTO(err_ret, ret);
#endif

        ret = md_chunk_set(NULL, &chkid, &nid, status);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int __lich_move_md(const fileid_t *fileid, const nid_t *dist)
{
        int ret, idx, i;
        chkinfo_t *chkinfo;
        char buf[MAX_BUF_LEN], str[MAX_BUF_LEN];
        nid_t nid[LICH_REPLICA_MAX * 2];

        chkinfo = (void *)buf;
        ret = md_chunk_getinfo(NULL, fileid, chkinfo, NULL);
        if (ret)
                GOTO(err_ret, ret);

        idx = -1;
        for (i = 0; i < chkinfo->repnum; i++) {
                if (nid_cmp(&chkinfo->diskid[i].id, dist) == 0) {
                        idx = i;
                }

                nid[i] = chkinfo->diskid[i].id;
        }

        if (idx == -1 || idx == 0) {
                nid[0] = *dist;
        } else {
                nid[idx] = nid[0];
                nid[0] = *dist;
        }

        metadata_check_diskid(nid, chkinfo->repnum);

        ret = md_move(&chkinfo->diskid[0].id, fileid, nid, chkinfo->repnum);
        if (ret)
                GOTO(err_ret, ret);

        ret = md_chunk_getinfo(NULL, fileid, chkinfo, NULL);
        if (ret)
                GOTO(err_ret, ret);

        CHKINFO_STR(chkinfo, str);
        printf("%s\n", str);

        return 0;
err_ret:
        return ret;
}

static int __lich_move_raw(const nid_t *nid, const fileid_t *fileid)
{
        int ret, chknum, i, retry = 0;
        chkinfo_t *_chkinfo;
        fileid_t chkid;
        fileinfo_t fileinfo;
        char tmp[MAX_BUF_LEN], str[MAX_BUF_LEN];

retry:
        ret = md_getattr(fileid, &fileinfo);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        _chkinfo = (void *)tmp;
        chknum = size2chknum(fileinfo.size);
        for (i = 0; i < chknum; i++) {
                retry = 0;
        retry1:
                ret = md_localize(nid, fileid, i);
                if (ret) {
                        if (ret == ENOENT) {
                                continue;
                        } else if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry1, retry);
                        } else
                                GOTO(err_ret, ret);
                }

                fid2cid(&chkid, fileid, i);
        retry2:
                ret = md_chunk_getinfo(NULL, &chkid, _chkinfo, NULL);
                if (ret) {
                        if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry2, retry);
                        } else {
                                GOTO(err_ret, ret);
                        }
                }

                CHKINFO_STR(_chkinfo, str);
                printf("%s\n", str);
        }

        return 0;
err_ret:
        return ret;
}

static int __lich_move(const char *path, const char *node, int async)
{
        int ret, retry = 0;
        nid_t nid;
        fileid_t fileid;

        (void) async;

        printf("move %s to %s\n", path, node);

        ret = __lich_getid(path, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        if (fileid.type != __VOLUME_CHUNK__) {
                EXIT(EINVAL);
        }

        ret = network_connect_byname(node, &nid);
        if (ret)
                GOTO(err_ret, ret);

retry:
        ret = __lich_move_md(&fileid, &nid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = __lich_move_raw(&nid, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

void * __lich_allocate_thread(void *_arg)
{
        int i, ret, retry;
        allocate_arg_t *arg;
        fileid_t chkid;
        chkinfo_t *chkinfo;
        char buf[MAX_BUF_LEN], str[MAX_BUF_LEN];

        arg = _arg;

        for (i = arg->begin; i < arg->end; i++) {
                retry = 0;
        retry1:
                fid2cid(&chkid, &arg->fileid, i);
                ret = md_chunk_allocate(&arg->fileid, &chkid, NULL);
                if (ret) {
                        if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry1, retry);
                        } else
                                GOTO(err_ret, ret);
                }

                retry = 0;
        retry2:
                chkinfo = (void *)buf;
                ret = md_chunk_getinfo(NULL, &chkid, chkinfo, NULL);
                if (ret) {
                        if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry2, retry);
                        } else {
                                GOTO(err_ret, ret);
                        }
                }

                CHKINFO_STR(chkinfo, str);
                printf("%s\n", str);
        }

        arg->ret = 0;
        return NULL;
err_ret:
        arg->ret = ret;
        return NULL;
}

static int __lich_allocate(const char *path, int multithreading)
{
        int ret, retry = 0, chknum, left, avg_size, size, i, count, begin, thread;
        fileid_t fileid;
        fileinfo_t fileinfo;
        allocate_arg_t args[ALLOCATE_THREAD_MAX];
        pthread_t ths[ALLOCATE_THREAD_MAX];

        printf("allocate %s\n", path);

        ret = __lich_getid(path, &fileid);
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

        if (multithreading) {
                thread = ALLOCATE_THREAD_MAX;
        } else {
                thread = 1;
        }

        chknum = size2chknum(fileinfo.size);
        left = chknum;
        avg_size = left % thread == 0 ? (left / thread) : (left / thread + 1);
        count = 0;
        begin = 0;
        while (left > 0) {
                size = left < avg_size ? left : avg_size;
                args[count].begin = begin;
                args[count].end = begin + size;
                args[count].ret = 0;
                args[count].fileid = fileid;

                count++;
                begin += size;
                left -= size;
        }

        for (i=0; i< count; i++) {
                ret = pthread_create(&ths[i], NULL, __lich_allocate_thread, &args[i]);
                if (ret)
                        GOTO(err_ret, ret);
        }

        for (i=0; i < count; i++) {
                pthread_join(ths[i], NULL);
        }

        for (i=0; i < count; i++) {
                ret = args[i].ret;
                if (ret) {
                        GOTO(err_ret, ret);
                }
        }

        return 0;
err_ret:
        return ret;
}

static int __lich_stat(const char *path, int verbose)
{
        int ret;
        fileid_t fileid;
        chkinfo_t *chkinfo;
        char _chkinfo[CHKINFO_MAX];
        filestat_t filestat;

        (void) verbose;

        ret = __lich_getid(path, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        chkinfo = (void *)_chkinfo;
        ret = md_chunk_getinfo(NULL, &fileid, chkinfo, NULL);
        if (ret)
                GOTO(err_ret, ret);

        ret = stor_rpc_stat(&chkinfo->diskid[0].id, &fileid, &filestat);
        if (ret)
                GOTO(err_ret, ret);

        printf("location : %s\n"
               "snap_version : %lu\n"
               "snap_rollback : %lu\n"
               "chknum : %u\n"
               "allocated : %u\n"
               "localized : %u\n"
               "rollback : %u\n",
               network_rname(&chkinfo->diskid[0].id),
               filestat.snap_version,
               filestat.snap_rollback,
               filestat.chknum,
               filestat.chknum - filestat.sparse,
               filestat.localized,
               filestat.rollback);
        if (filestat.attr & __FILE_ATTR_CLONE__) {
                printf("source : %s\n", filestat.src);
        }

        return 0;
err_ret:
        return ret;
}

static int __lookup_cmp(chkinfo_t *chkinfo, const fileid_t *findid, const char *path)
{
        fileid_t *fileid;
        char info[MAX_BUF_LEN];
        int i;
        const char *stat;
        const reploc_t *diskid;

        fileid = &chkinfo->id;
        if (!chkid_cmp(fileid, findid)) {
                __lookup_find = 1;

                info[0] = '\0';
                for (i = 0; i < (int)chkinfo->repnum; ++i) {
                        diskid = &chkinfo->diskid[i];
                        network_connect1(&diskid->id);
                        if (netable_connected(&diskid->id) == 0) {
                                stat = "offline";
                        } else if (diskid->status == __S_DIRTY) {
                                stat = "dirty";
                        } else if (diskid->status == __S_CHECK) {
                                stat = "check";
                        } else {
                                stat = "clean";
                        }
                        snprintf(info + strlen(info), MAX_NAME_LEN, "%s:%s",
                                 network_rname(&diskid->id), stat);
                        if (i != (int)chkinfo->repnum - 1)
                                strcat(info, ", ");
                }

                printf("chkid : %s\n", id2str(&chkinfo->id));
                printf("path : %s\n", path);
                printf("infover : %llu\n", (LLU)chkinfo->info_version);
                printf("repnum : %d\n", chkinfo->repnum);
                printf("chkinfo : [%s]\n", info);

                return 0;
        }

        return 1;
}

static int __lookup_file(const fileid_t *fileid, const fileid_t *findid, const char *_path)
{
        int ret, delen, retry = 0;
        char buf[MAX_BUF_LEN], de0[BIG_BUF_LEN], path[MAX_PATH_LEN];
        chkinfo_t *chkinfo;
        struct dirent *de;
        uint64_t offset;

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

                if (__lookup_find)
                        goto out;

                chkinfo = (void *)buf;
                ret = md_lookup_byname(fileid, de->d_name, chkinfo, NULL);
                if (ret)
                        GOTO(err_ret, ret);

                sprintf(path, "%s@%s", _path, de->d_name);

                if (!__lookup_cmp(chkinfo, findid, path))
                        goto out;

        }

out:
        return 0;
err_ret:
        return ret;
}

static int __lookup_dir(const fileid_t *fileid, const fileid_t *findid, const char *_path)
{
        int ret, delen, retry = 0;
        struct dirent *de;
        char buf[MAX_BUF_LEN], de0[BIG_BUF_LEN], path[MAX_PATH_LEN];
        uint64_t offset;
        chkinfo_t *chkinfo;

        YASSERT(fileid->type == __POOL_CHUNK__);

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

                        if (__lookup_find)
                                goto out;

                        chkinfo = (void *)buf;
                        ret = md_lookup_byname(fileid, de->d_name, chkinfo, NULL);
                        if (ret)
                                GOTO(err_ret, ret);

                        if (!strcmp(_path, "/"))
                                sprintf(path, "/%s", de->d_name);
                        else
                                sprintf(path, "%s/%s", _path, de->d_name);

                        if (chkinfo->id.type == __VOLUME_CHUNK__) {
                                if (!__lookup_cmp(chkinfo, findid, path))
                                        goto out;

                                if (!chkid_cmp(&unlink_fid, fileid))
                                        continue;

                                ret = __lookup_file(&chkinfo->id, findid, path);
                                if (ret)
                                        GOTO(err_ret, ret);
                        } else {
                                if (!__lookup_cmp(chkinfo, findid, path))
                                        goto out;

                                ret = __lookup_dir(&chkinfo->id, findid, path);
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

static int __lich_lookup(const char *name)
{
        int ret;
        fileid_t rootid;
        fileid_t findid;
        chkinfo_t *chkinfo;
        char buf[MAX_BUF_LEN];

        ret = __lich_getid(name, &findid);
        if (ret)
                GOTO(err_ret, ret);

        ret = stor_lookup1("/", &rootid);
        if (ret)
                GOTO(err_ret, ret);

        ret = stor_lookup1(UNLINK_ROOT, &unlink_fid);
        if (ret)
                GOTO(err_ret, ret);

        chkinfo = (void *)buf;
        ret = md_chunk_getinfo(NULL, &rootid, chkinfo, NULL);
        if (ret)
                GOTO(err_ret, ret);

        if (!__lookup_cmp(chkinfo, &findid, "/"))
                goto out;

        ret = __lookup_dir(&rootid, &findid, "/");
        if (ret)
                GOTO(err_ret, ret);

        if (!__lookup_find)
                printf("not found!\n");
out:
        return 0;
err_ret:
        return ret;
}

static int __lich_paxos_dump()
{
        char path[MAX_PATH_LEN];

        snprintf(path, MAX_PATH_LEN, "%s/data/node/paxos/", gloconf.home);

        return paxos_dump(path);
}

static int __lich_md5sum(const char *name)
{
        int ret;
        fileid_t fileid;

        ret = __lich_getid(name, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        ret = file_md5sum(&fileid);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int __lich_chunk_move(const char *path, const char *to)
{
        int ret, dist_count, i;
        char tmp[MAX_BUF_LEN];
        char *tmp2[LICH_REPLICA_MAX];
        nid_t dist[LICH_REPLICA_MAX];
        fileid_t _parent;
        chkid_t _chkid;
        fileid_t *parent = &_parent;
        chkid_t *chkid = &_chkid;
        chkinfo_t *chkinfo, *parent_chkinfo;
        char buf[CHKINFO_MAX];
        char buf2[CHKINFO_MAX];
        chkinfo = (void *)buf;
        parent_chkinfo = (void *)buf2;
        nid_t nid;
        fileid_t srv;

        if (strlen(path) == 0) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        ret = __lich_getid(path, chkid);
        if (ret)
                GOTO(err_ret, ret);

        strcpy(tmp, to);
        dist_count = LICH_REPLICA_MAX;
        _str_split(tmp, ',', tmp2, &dist_count);

        for (i = 0; i < dist_count; i++) {
                ret = network_connect_byname(tmp2[i], &dist[i]);
                if (ret)
                        GOTO(err_ret, ret);
        }

        ret = md_parent_get(chkid, parent);
        if (ret)
                GOTO(err_ret, ret);

        ret = md_chunk_getinfo(parent, chkid, chkinfo, NULL);
        if (ret)
                GOTO(err_ret, ret);

        if (chkid->type == __POOL_CHUNK__ || chkid->type == __VOLUME_CHUNK__) {
                nid = chkinfo->diskid[0].id;
                srv = chkinfo->id;
        } else {
                ret = md_chunk_getinfo(NULL, parent, parent_chkinfo, NULL);
                if (ret)
                        GOTO(err_ret, ret);

                nid = parent_chkinfo->diskid[0].id;
                srv = *parent;
        }

        ret = md_chunk_move(&nid, &srv, chkid, (const nid_t *)dist, dist_count);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int __lich_chunk_stat(const char *path)
{
        int ret;
        fileid_t _parent;
        chkid_t _chkid;
        fileid_t *parent = &_parent;
        chkid_t *chkid= &_chkid;
        chkinfo_t *chkinfo, *parent_chkinfo;
        char buf[CHKINFO_MAX], buf2[CHKINFO_MAX], buf3[MAX_BUF_LEN];
        chkinfo = (void *)buf;
        parent_chkinfo = (void *)buf2;

        if (strlen(path) == 0) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        ret = __lich_getid(path, chkid);
        if (ret)
                GOTO(err_ret, ret);

        ret = md_parent_get(chkid, parent);
        if (ret)
                GOTO(err_ret, ret);

        ret = md_chunk_getinfo(parent, chkid, chkinfo, NULL);
        if (ret)
                GOTO(err_ret, ret);

        if (chkid_isnull(parent)) {
                printf("parent: null\n");
        } else {
                ret = md_chunk_getinfo(NULL, parent, parent_chkinfo, NULL);
                if (ret)
                        GOTO(err_ret, ret);

                CHKINFO_STR(parent_chkinfo, buf3);
                printf("parent: %s\n", buf3);
        }
        CHKINFO_STR(chkinfo, buf3);
        printf("chkinfo: %s\n", buf3);

        return 0;
err_ret:
        return ret;
}

static int __db_dump__(const char *path, char *sql, int *count, int *disk, int *loc)
{
        int ret, row, column, i, r, slen;
        sqlite3 *db;
        char *zErrMsg = NULL, **result;
        char key[MAX_BUF_LEN], _parent[MAX_BUF_LEN];
        chkid_t *chkid, *parent;

        ret = sqlite3_open(path, &db);
        if (ret ){
                ret = EIO;
                GOTO(err_ret, ret);
        }

        ret = sqlite3_get_table(db, sql, &result, &row, &column, &zErrMsg);
        if (ret != SQLITE_OK ){
                DERROR("SQL error: %s\n", zErrMsg);
                GOTO(err_close, ret);
        }

        r = column;
        for (i = 0; i < row; i++) {
                base64_decode(result[r], &slen, key);
                chkid = (void *)key;

                base64_decode(result[r+3], &slen, _parent);
                parent = (void *)_parent;

                printf(""CHKID_FORMAT", %s, %s, "CHKID_FORMAT", %s, %s\n",
                                CHKID_ARG(chkid), result[r+1], result[r+2], CHKID_ARG(parent),
                                result[r+4], result[r+5]);
                if (count)
                        *count = *count + 1;
                if (disk)
                        *disk = atoi(result[r+1]);
                if (loc)
                        *loc = atoi(result[r+2]);
                r = r + column;
        }
        /*printf("column: %d, row: %d\n", column, row);*/

        sqlite3_free_table(result);
        sqlite3_free(zErrMsg);

        ret = sqlite3_close(db);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_close:
        YASSERT(sqlite3_close(db));
err_ret:
        return ret;
}

#define __DB_CHUNK_COUNT__ 10

/*临时用来查数据库用*/
static int __lich_db_dump(const char *idx)
{
        int ret, i;
        char sql[MAX_BUF_LEN], path[MAX_PATH_LEN];

        if (!strcmp(idx, "all")) {
                sprintf(sql, "select * from metadata");
                for (i = 0; i < __DB_CHUNK_COUNT__; i++) {
                        sprintf(path, "%s/data/chunk/%d.db", gloconf.home, i);
                        __db_dump__(path, sql, NULL, NULL, NULL);
                }

                sprintf(sql, "select * from raw");
                for (i = 0; i < __DB_CHUNK_COUNT__; i++) {
                        sprintf(path, "%s/data/chunk/%d.db", gloconf.home, i);
                        __db_dump__(path, sql, NULL, NULL, NULL);
                }
        } else if (strlen(idx) == 1 && isdigit(*idx) && atoi(idx) >= 0 && atoi(idx) < __DB_CHUNK_COUNT__) {
                sprintf(path, "%s/data/chunk/%s.db", gloconf.home, idx);

                sprintf(sql, "select * from metadata");
                __db_dump__(path, sql, NULL, NULL, NULL);

                sprintf(sql, "select * from raw");
                __db_dump__(path, sql, NULL, NULL, NULL);
        } else {
                DERROR("%s/data/chunk/%s.db not found!\n", gloconf.home, idx)
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __diskmd_real_path(char *path, struct stat* stbuf)
{
        int ret;
        char buf[MAX_PATH_LEN];

        ret = lstat(path, stbuf);
        if (ret < 0) {
                ret = errno;
                goto err_ret;
        }

        while (S_ISLNK(stbuf->st_mode)) {
                memset(buf, 0, sizeof(buf));

                ret = readlink(path, buf, sizeof(buf));
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }

                realpath(buf, path);

                ret = lstat(path, stbuf);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }
        }

        return 0;
err_ret:
        return ret;
}

static void __db_select(const fileid_t *fileid, int *count, int *disk, int *loc)
{
        int i;
        char sql[MAX_BUF_LEN], key[MAX_BUF_LEN], path[MAX_PATH_LEN];

        base64_encode((void *)fileid, sizeof(*fileid), key);
        *count = 0;

        sprintf(sql, "select * from metadata where key='%s'", key);
        for (i = 0; i < __DB_CHUNK_COUNT__; i++) {
                sprintf(path, "%s/data/chunk/%d.db", gloconf.home, i);
                __db_dump__(path, sql, count, disk, loc);
        }

        sprintf(sql, "select * from raw where key='%s'", key);
        for (i = 0; i < __DB_CHUNK_COUNT__; i++) {
                sprintf(path, "%s/data/chunk/%d.db", gloconf.home, i);
                __db_dump__(path, sql, count, disk, loc);
        }

        YASSERT(*count <= 1);
}

static int __lich_chunk_dump(const char *path, const char *dst)
{
        int ret, found, disk, loc, fd;
        fileid_t fileid;
        char diskpath[PATH_MAX], buf[LICH_CHUNK_SPLIT];
        struct stat stbuf;


        ret = __lich_getid(path, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        __db_select(&fileid, &found, &disk, &loc);

        if (found) {
                sprintf(diskpath, "%s/data/disk/disk/%d.disk", gloconf.home, disk);

                ret = __diskmd_real_path(diskpath, &stbuf);
                if (ret) {
                        GOTO(err_ret, ret);
                }

                fd = open(diskpath, O_RDWR | O_SYNC, 0);
                if (fd < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }

                ret = pread(fd, buf, LICH_CHUNK_SPLIT, loc * LICH_CHUNK_SPLIT);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_fd, ret);
                }

                close(fd);

                if (dst)
                        snprintf(diskpath, PATH_MAX, "%s", dst);
                else
                        sprintf(diskpath, ""CHKID_FORMAT".dump", CHKID_ARG(&fileid));

                fd = open(diskpath, O_CREAT | O_RDWR | O_SYNC, 0644);
                if (fd < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }

                printf("dump chunk "CHKID_FORMAT" to %s\n", CHKID_ARG(&fileid), diskpath);

                ret = write(fd, buf, LICH_CHUNK_SPLIT);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_fd, ret);
                }

                close(fd);
        } else {
                printf("chkid "CHKID_FORMAT" not found!\n", CHKID_ARG(&fileid));
        }

        return 0;
err_fd:
        close(fd);
err_ret:
        return ret;
}

static int __lich_chunk_zerofill(const char *path)
{
        int ret, found, disk, loc, fd;
        fileid_t fileid;
        char diskpath[PATH_MAX], buf[LICH_CHUNK_SPLIT];
        struct stat stbuf;


        ret = __lich_getid(path, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        __db_select(&fileid, &found, &disk, &loc);

        if (found) {
                printf("fill zero to chunk "CHKID_FORMAT"\n", CHKID_ARG(&fileid));

                sprintf(diskpath, "%s/data/disk/disk/%d.disk", gloconf.home, disk);

                ret = __diskmd_real_path(diskpath, &stbuf);
                if (ret) {
                        GOTO(err_ret, ret);
                }

                fd = open(diskpath, O_RDWR | O_SYNC, 0);
                if (fd < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }

                memset(buf, 0x0, sizeof(buf));

                ret = pwrite(fd, buf, LICH_CHUNK_SPLIT, loc * LICH_CHUNK_SPLIT);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_fd, ret);
                }

                close(fd);
        } else {
                printf("chkid "CHKID_FORMAT" not found!\n", CHKID_ARG(&fileid));
        }

        return 0;
err_fd:
        close(fd);
err_ret:
        return ret;
}

static int __lich_chunk_fill(const char *path, const char *src)
{
        int ret, found, disk, loc, fd;
        fileid_t fileid;
        char diskpath[PATH_MAX], buf[LICH_CHUNK_SPLIT];
        struct stat stbuf;


        ret = __lich_getid(path, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        __db_select(&fileid, &found, &disk, &loc);

        if (found) {
                printf("fill %s to chunk "CHKID_FORMAT"\n", src, CHKID_ARG(&fileid));

                fd = open(src, O_CREAT | O_RDWR | O_SYNC, 0644);
                if (fd < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }

                ret = read(fd, buf, LICH_CHUNK_SPLIT);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_fd, ret);
                }

                close(fd);

                sprintf(diskpath, "%s/data/disk/disk/%d.disk", gloconf.home, disk);

                ret = __diskmd_real_path(diskpath, &stbuf);
                if (ret) {
                        GOTO(err_ret, ret);
                }

                fd = open(diskpath, O_RDWR | O_SYNC, 0);
                if (fd < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }

                ret = pwrite(fd, buf, LICH_CHUNK_SPLIT, loc * LICH_CHUNK_SPLIT);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_fd, ret);
                }

                close(fd);
        } else {
                printf("chkid "CHKID_FORMAT" not found!\n", CHKID_ARG(&fileid));
        }

        return 0;
err_fd:
        close(fd);
err_ret:
        return ret;
}

static int __lich_base64_encode(const char *name, int pithy)
{
        int ret;
        fileid_t fileid;
        char key[MAX_BUF_LEN];

        if (pithy) {
                base64_encode((void *)name, strlen(name), key);
                printf("encode: %s\n", key);
        } else {
                ret = __lich_getid(name, &fileid);
                if (ret)
                        GOTO(err_ret, ret);

                base64_encode((void *)&fileid, sizeof(fileid), key);
                printf("chkid: "CHKID_FORMAT"\n", CHKID_ARG(&fileid));
                printf("encode: %s\n", key);
        }

        return 0;
err_ret:
        return ret;
}

static int __lich_base64_decode(const char *name, int pithy)
{
        int slen, i;
        char key[MAX_BUF_LEN];
        chkid_t *chkid;

        if (pithy) {
                base64_decode(name, &slen, key);
                printf("decode: %s\n   HEX:", key);
                for (i=0; i<slen; i++) {
                        printf("%x ", key[i]);
                }
                printf("(%d)\n", slen);
        } else {
                base64_decode(name, &slen, key);
                chkid = (void *)key;
                printf("chkid: "CHKID_FORMAT"\n", CHKID_ARG(chkid));
        }

        return 0;
}

int main(int argc, char *argv[])
{
        int ret, op = OP_NULL, alone = 0, verbose = 0, force = 0,
                async = 0, i, pithy = 0, deep = 0, multithreading = 1;
        char c_opt, cwdpath[PATH_MAX];
        const char *path = NULL, *name = NULL, *to = NULL;

        dbg_info(0);

        (void) async;
        (void) force;
        (void) path;

        while (srv_running) {
                int option_index = 0;

                static struct option long_options[] = {
                        { "cmp", required_argument, 0, 0},
                        { "recover", required_argument, 0, 0},
                        { "chunkset", required_argument, 0, 0},
                        { "async", required_argument, 0, 0},
                        { "allocate", required_argument, 0, 0},
                        { "scan", required_argument, 0, 0},
                        { "paxosdump", no_argument, 0, 0},
                        { "lookup", required_argument, 0, 0},
                        { "deep", no_argument, 0, 0},
                        { "md5sum", required_argument, 0, 0},
                        { "tier", required_argument, 0, 0},
                        { "priority", required_argument, 0, 0},
                        { "localize", required_argument, 0, 0},
                        { "multpath", required_argument, 0, 0},
                        { "chunkmove", required_argument, 0, 0},
                        { "chunkstat", required_argument, 0, 0},
                        { "dbdump", required_argument, 0, 0},
                        { "base64", required_argument, 0, 0},
                        { "single", no_argument, 0, 0},
                        { "chunkdump", required_argument, 0, 0},
                        { "chunkfill", required_argument, 0, 0},
                        { "list", required_argument, 0, 'l'},
                        { "chunkinfo", required_argument, 0, 'd'},
                        { "move", required_argument, 0, 'm'},
                        { "stat", required_argument, 0, 's'},
                        { "pithy", 0, 0, 'p'},
                        { "verbose", 0, 0, 'v' },
                        { "help",    0, 0, 'h' },
                        { 0, 0, 0, 0 },
                };

                c_opt = getopt_long(argc, argv, "vhl:s:pc:qo:", long_options, &option_index);
                if (c_opt == -1)
                        break;

                switch (c_opt) {
                case 0:
                        switch (option_index) {
                        case 0:
                                DBUG("cmp\n");
                                op = OP_CMP;
                                path = optarg;
                                break;
                        case 1:
                                DBUG("recover\n");
                                op = OP_RECOVER;
                                path = optarg;
                                break;
                        case 2:
                                op = OP_CHUNKSET;
                                path = optarg;
                                break;
                        case 3:
                                async = 1;
                                break;
                        case 4:
                                op = OP_ALLOCATE;
                                path = optarg;
                                break;
                        case 5:
                                op = OP_SCAN;
                                path = optarg;

                                break;
                        case 6:
                                op = OP_PAXOSDUMP;
                                alone = 1;
                                break;
                        case 7:
                                op = OP_LOOKUP;
                                name = optarg;
                                break;
                        case 8:
                                deep = 1;
                                break;
                        case 9:
                                op = OP_MD5SUM;
                                path = optarg;
                                break;
                        case 10:
                                op = OP_TIER;
                                path = optarg;
                                break;
                        case 11:
                                op = OP_PRIORITY;
                                path = optarg;
                                break;
                        case 12:
                                op = OP_LOCALIZE;
                                path = optarg;
                                break;
                        case 13:
                                op = OP_MULTPATH;
                                path = optarg;
                                break;
                        case 14:
                                op = OP_CHUNK_MOVE;

                                if (argc < 4) {
                                        usage();
                                        exit(EINVAL);
                                }

                                for (i = 0; i < argc; i++) {
                                        if (optarg == argv[i]) {
                                                path = argv[i];
                                                to = argv[i + 1];
                                        }
                                }
                                break;
                        case 15:
                                op = OP_CHUNK_STAT;
                                path = optarg;
                                break;
                        case 16:
                                op = OP_DB_DUMP;
                                path = optarg;
                                break;
                        case 17:
                                op = OP_BASE64;
                                name = optarg;
                                break;
                        case 18:
                                multithreading = 0;
                                break;
                        case 19:
                                op = OP_CHUNK_DUMP;
                                path = optarg;
                                break;
                        case 20:
                                op = OP_CHUNK_FILL;
                                path = optarg;
                                break;
                        default:
                                fprintf(stderr, "Hoops, wrong op got!\n");
                                YASSERT(0); 
                        }

                        break;
                case 'l':
                        DBUG("list\n");
                        op = OP_LIST;
                        path = optarg;
                        break;
                case 's':
                        DBUG("stat\n");
                        op = OP_STAT;
                        path = optarg;
                        break;
                case 'm':
                        DBUG("move %s\n", argv[1]);
                        op = OP_MOVE;

                        //args = (void *)optarg;

                        if (argc < 4) {
                                usage();
                                EXIT(EINVAL);
                        }

                        for (i = 0; i < argc; i++) {
                                if (optarg == argv[i]) {
                                        path = argv[i];
                                        to = argv[i + 1];
                                }
                        }

                        break;
                case 'b':
                        op = OP_BLANCEOBJ;
                        path = optarg;
                        break;
                case 'd':
                        DBUG("stat\n");
                        op = OP_CHUNK_INFO;
                        path = optarg;
                        break;
                case 'c':
                        DBUG("connection\n");
                        op = OP_CONNECTION;
                        path = optarg;
                        break;
                case 'p':
                        pithy = 1;
                        break;
                case 'v':
                        verbose = 1;
                        break;
                case 'h':
                        usage();
                        EXIT(0);
                default:
                        usage();
                        EXIT(EINVAL);
                }
        }

        getcwd(cwdpath, PATH_MAX);
#if 1
        if (argc == 1) {
                usage();
                exit(EINVAL);
        }
#endif

        if (op == OP_PAXOSDUMP || op == OP_BASE64) {
                ret = env_init_simple("lich.inspect");
                if (ret)
                        GOTO(err_ret, ret);

                ng.offline = 1;
        } else {
                ret = __inspect_init(op);
                if (ret && alone == 0) {
                        GOTO(err_ret, ret);
                }
        } 

        switch (op) {
        case OP_LIST:
                ret = __lich_list(path);
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_CHUNK_INFO:
                ret = __lich_dump(path, verbose, pithy);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_CMP:
                ret = __lich_cmp(path);
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_RECOVER:
                ret = recovery_bypath(path, deep);
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_CHUNKSET:
                ret = __lich_chunkset(path, argv[3], argv[4]);
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_MOVE:
                ret = __lich_move(path, to, async);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_ALLOCATE:
                ret = __lich_allocate(path, multithreading);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_STAT:
                ret = __lich_stat(path, verbose);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_LOOKUP:
                ret = __lich_lookup(name);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_PAXOSDUMP:
                ret = __lich_paxos_dump();
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_SCAN:
                ret = recovery_scan(path);
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_MD5SUM:
                ret = __lich_md5sum(path);
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_TIER:
                if ((!verbose && argc == 4) || argc == 5) {
                        int num;
                        if (verbose)
                                num = atoi(argv[4]);
                        else
                                num = atoi(argv[3]);
                        
                        ret = lich_tier_set(path, num, verbose);
                        if (ret)
                                GOTO(err_ret, ret);
                } else {
                        ret = lich_tier_get(path, verbose);
                        if (ret)
                                GOTO(err_ret, ret);
                }

                break;
        case OP_PRIORITY:
                if ((!pithy && !verbose && argc == 4) || argc == 5) {
                        int num;
                        if (verbose)
                                num = strcmp(argv[4], "default") ? atoi(argv[4]) : -1;
                        else
                                num = strcmp(argv[3], "default") ? atoi(argv[3]) : -1;

                        pithy = 1;
                        ret = lich_priority_set(path, num, verbose, pithy);
                        if (ret)
                                GOTO(err_ret, ret);
                } else {
                        pithy = 1;
                        ret = lich_priority_get(path, verbose, pithy);
                        if (ret)
                                GOTO(err_ret, ret);
                }

                break;
        case OP_LOCALIZE:
                if (argc == 4) {
                        int num;
                        num = atoi(argv[3]);
                        ret = lich_localize_set(path, num);
                        if (ret)
                                GOTO(err_ret, ret);
                } else if (argc == 3) {
                        ret = lich_localize_get(path);
                        if (ret)
                                GOTO(err_ret, ret);
                } else {
                        ret = EINVAL;
                        GOTO(err_ret, ret);
                }

                break;
        case OP_MULTPATH:
                if (argc == 4) {
                        int num;
                        num = atoi(argv[3]);
                        ret = lich_multpath_set(path, num);
                        if (ret)
                                GOTO(err_ret, ret);
                } else if (argc == 3) {
                        ret = lich_multpath_get(path);
                        if (ret)
                                GOTO(err_ret, ret);
                } else {
                        GOTO(err_ret, ret);
                }

                break;
        case OP_CHUNK_MOVE:
                ret = __lich_chunk_move(path, to);
                if (ret)
                        goto err_ret;

                break;
        case OP_CHUNK_STAT:
                ret = __lich_chunk_stat(path);
                if (ret)
                        goto err_ret;

                break;
        case OP_DB_DUMP:
                ret = __lich_db_dump(path);
                if (ret)
                        goto err_ret;

                break;
        case OP_BASE64:
                if (argv[3] == NULL) {
                        usage();
                        EXIT(EINVAL);
                }

                if (!strncmp(name, "encode", strlen(name))) {
                        if (pithy) {
                                to = argv[4];
                        } else {
                                to = argv[3];
                        }

                        ret = __lich_base64_encode(to, pithy);
                        if (ret)
                                GOTO(err_ret, ret);
                } else if (!strncmp(name, "decode", strlen(name))) {
                        if (pithy) {
                                to = argv[4];
                        } else {
                                to = argv[3];
                        }

                        ret = __lich_base64_decode(to, pithy);
                        if (ret)
                                GOTO(err_ret, ret);
                } else {
                        usage();
                        EXIT(EINVAL);
                }

                break;
        case OP_CHUNK_DUMP:
                chdir(cwdpath);

                if (argc == 3) {
                        ret = __lich_chunk_dump(path, NULL);
                        if (ret)
                                goto err_ret;
                } else if (argc == 4) {
                        ret = __lich_chunk_dump(path, argv[3]);
                        if (ret)
                                goto err_ret;
                } else {
                        usage();
                        EXIT(EINVAL);
                }


                break;
        case OP_CHUNK_FILL:
                chdir(cwdpath);
                if (argc == 3) {
                        ret = __lich_chunk_zerofill(path);
                        if (ret)
                                goto err_ret;
                } else if (argc == 4) {
                        ret = __lich_chunk_fill(path, argv[3]);
                        if (ret)
                                goto err_ret;
                } else {
                        usage();
                        EXIT(EINVAL);
                }

                break;
        default:
                usage();
                EXIT(EINVAL);
        }

        //DBUG("test...........\n");

        return 0;
err_ret:
        EXIT(_errno(ret));
}
