#include "config.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <dirent.h>
#include <ctype.h>
#include <libgen.h>

#define DBG_SUBSYS S_LIBINTERFACE

#include "configure.h"
// #include "env.h"
#include "adt.h"
#include "net_table.h"
#include "lichstor.h"
#include "cluster.h"
#include "metadata.h"
#include "lich_md.h"
#include "storage.h"
#include "volume.h"
#include "lichbd.h"
#include "license.h"
#include "md_map.h"
#include "net_global.h"
#include "utils.h"
#include "dbg.h"

int object_md5sum(const fileid_t *oid);

#define MULTITHREADING          1
#define COPY_THREAD_MAX         10
#define COPY_SIZE_MIN           32      /* 32M */

typedef struct {
        int fd;
        fileid_t fileid;
        fileid_t snapid;
        void * (*worker)(void *_arg);
} arg_ext_t;

typedef struct {
        sem_t sem;
        int ret;
        arg_ext_t ext;
        off_t offset;
        size_t size;
} arg_t;

int utils_get_size(const char *_str, uint64_t *_size)
{
        int ret;
        uint64_t size = 0;
        char unit, str[MAX_NAME_LEN];

        if (strlen(_str) < 2) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        strcpy(str, _str);

        unit = str[strlen(str) - 1];
        str[strlen(str) - 1] = 0;

        size = atoll(str);

        switch (unit) {
        case 'b':
        case 'B':
                break;
        case 'k':
        case 'K':
                size *= 1000;
                break;
        case 'm':
        case 'M':
                size *= (1000 * 1000);
                break;
        case 'g':
        case 'G':
                size *= (1000 * 1000 * 1000);
                break;
        default:
                fprintf(stderr, "size unit must be specified, see help for detail\n");
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        *_size = size;

        return 0;
err_ret:
        return ret;
}

int utils_mkpool(const char *dir)
{
        int ret, retry = 0;
        char name[MAX_NAME_LEN];
        fileid_t fid;

retry0:
        ret = stor_splitpath(dir, &fid, name);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

retry:
        ret = stor_mkpool(&fid, name, NULL, NULL);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else if (ret == EEXIST) {
                        //nothing todo
                        goto err_ret;
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

int utils_rmpool(const char *dir)
{
        int ret, retry = 0;
        char name[MAX_NAME_LEN];
        fileid_t fid;

retry:
        ret = stor_splitpath(dir, &fid, name);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        retry = 0;
retry1:
        ret = stor_rmpool(&fid, name);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry1, retry);
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

int utils_touch(const char *file)
{
        int ret, retry = 0;
        char name[MAX_NAME_LEN];
        fileid_t parentid, fid;

retry0:
        ret = stor_splitpath(file, &parentid, name);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

retry:
        ret = stor_mkvol(&parentid, name, NULL, &fid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else if (ret == EEXIST) {
                        //nothing todo
                        goto err_ret;
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

int utils_rmvol(const char *file)
{
        int ret, retry = 0;
        char name[MAX_NAME_LEN];
        fileid_t parent, fid;

retry0:
        ret = stor_splitpath(file, &parent, name);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

retry:
        ret = stor_lookup(&parent, name, &fid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        if (fid.type != __VOLUME_CHUNK__) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        retry = 0;
retry1:
        ret = stor_rmvol(&parent, name);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry1, retry);
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

int utils_statstr(const fileid_t *parent, const char *name, char *statstr)
{
        int ret, retry = 0;
        char stat[128], perm[128];
        fileid_t fid;
        struct stat stbuf;

retry0:
        ret = stor_lookup(parent, name, &fid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_getattr(&fid, &stbuf);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        stat_to_datestr(&stbuf, stat);
        mode_to_permstr(stbuf.st_mode, perm);

        sprintf(statstr, "%s %d %d %d %llu %s %s",
                perm, 1, stbuf.st_gid, stbuf.st_uid, (LLU)stbuf.st_size, stat, name);

        return 0;
err_ret:
        return ret;
}

void utils_statstr_error(const char *name, char *statstr)
{
        sprintf(statstr, "?????????? ? ? ? ? ??? ?? ??:?? %s", name);
}

/**
 * flag:  0 -- only show file
 *        1 -- only show dir
 *        other -- show all
 */
int utils_list(const char *dir, int flag)
{
        int ret, done = 0, retry = 0;
        off_t offset = 0;
        fileid_t fid;
        int delen;
        struct dirent *de0, *de;
        char statstr[MAX_PATH_LEN];

retry0:
        ret = stor_lookup1(dir, &fid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        while (done == 0) {
                ret = stor_listpool(&fid, offset, (void **)&de0, &delen);
                if (ret) {
                        if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry0, retry);
                        } else
                                GOTO(err_ret, ret);
                }

                if (delen == 0) {
                        done = 1;
                        break;
                }

                dir_for_each(de0, delen, de, offset) {
                        if (strlen(de->d_name) == 0) {
                                done = 1;
                                break;
                        }

                        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
                                continue;
                        }

                        memset(statstr, 0x00, MAX_PATH_LEN);
                        ret = utils_statstr(&fid, de->d_name, statstr);
                        if (ret) {
                                utils_statstr_error(de->d_name, statstr);
                        }

                        switch (flag) {
                                case 0:
                                        if (statstr[0] == '-') {
                                                printf("%s\n", statstr);
                                        }
                                        break;
                                case 1:
                                        if (statstr[0] == 'd') {
                                                printf("%s\n", statstr);
                                        }
                                        break;
                                default:
                                        printf("%s\n", statstr);
                                        break;
                        }

                }

                yfree((void **)&de0);
        }

        return 0;
err_ret:
        return ret;
}

int utils_find_list(const char *parent, const char *key)
{
        int ret, retry = 0;
        off_t offset = 0;
        fileid_t fid;
        char stat[128], perm[128];
        char name[MAX_NAME_LEN];
        struct stat stbuf;
        int delen;
        struct dirent *de0, *de;

retry0:
        ret = stor_lookup1(parent, &fid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_getattr(&fid, &stbuf);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        stat_to_datestr(&stbuf, stat);
        mode_to_permstr(stbuf.st_mode, perm);

        /*
         *printf("%s %d %d %d %llu %s --- %s\n",
         *                perm, 1, stbuf.st_gid, stbuf.st_uid, (LLU)stbuf.st_size, stat, parent);
         */

        if (key == NULL || !strcmp(key, basename((char *)parent))) {
                printf("%s\n", parent);
        }

        if (S_ISDIR(stbuf.st_mode)) {
                ret = stor_listpool(&fid, offset, (void **)&de0, &delen);

                dir_for_each(de0, delen, de, offset) {
                        if (strlen(de->d_name) == 0) {
                                break;
                        }

                        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
                                continue;
                        }

                        if (!strcmp(parent, "/")) {
                                sprintf(name, "%s%s", parent, de->d_name);
                        } else {
                                sprintf(name, "%s/%s", parent, de->d_name);
                        }

                        utils_find_list(name, key);
                }
        }

        return 0;
err_ret:
        return ret;
}

int utils_find(const char *path, const char *name)
{
        int ret;

        /*(void)name;*/

        ret = utils_find_list(path, name);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

int utils_rename(const char *from, const char *to)
{
        int ret, retry = 0;
        fileid_t fromdir, todir;
        char fromname[MAX_NAME_LEN], toname[MAX_NAME_LEN];

retry0:
        ret = stor_splitpath(from, &fromdir, fromname);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_splitpath(to, &todir, toname);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

retry:
        ret = stor_rename(&fromdir, fromname, &todir, toname);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

/**
 * format:   0  -- lich
 *           1  -- json
 *           2  -- xml
 */

int utils_stat(const char *path, int format)
{
        int ret, retry = 0;
        fileid_t fid;
        struct stat stbuf;
        char date[32], perm[32], type[32];

retry0:
        ret = stor_lookup1(path, &fid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_getattr(&fid, &stbuf);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        stat_to_datestr(&stbuf, date);
        mode_to_permstr(stbuf.st_mode, perm);

        if (S_ISDIR(stbuf.st_mode))
                strcpy(type, "directory");
        else if (S_ISREG(stbuf.st_mode))
                strcpy(type, "regular file");
        else
                sprintf(type, "unknown(%x)", stbuf.st_mode);

        if (format == 1) {
                printf("{\"name\":\"%s\",\"size\":%llu,\"objects\":3,\"order\":22,\"object_size\":%llu,", path, (LLU)stbuf.st_size, (LLU)stbuf.st_size);
                printf("\"block_name_prefix\":\"rbd_data.5e356b8b4567\",\"format\":2,");
                printf("\"features\":[\"layering\",\"exclusive-lock\",\"object-map\",\"fast-diff\",\"deep-flatten\"],\"flags\":[]}");
        } else {
                printf("  File: '%s' Id: "CHKID_FORMAT"\n"
                                "  Size: %llu %s\n"
                                "Access: (%o/%s) Uid: (%u) Gid: (%u)\n"
                                "Access: %s\n"
                                "Modify: %s\n"
                                "Change: %s\n"
                                " Birth: %s\n",
                                path, CHKID_ARG(&fid),
                                (LLU)stbuf.st_size, type,
                                stbuf.st_mode, perm, stbuf.st_uid, stbuf.st_gid,
                                date,
                                date,
                                date,
                                date);
        }

        return 0;
err_ret:
        return ret;
}

int utils_truncate(const char *path, size_t size)
{
        int ret, retry = 0;
        fileid_t fileid;

retry0:
        ret = stor_lookup1(path, &fileid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

retry:
        ret = stor_truncate(&fileid, size);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;        
}

int utils_cat(const char *path)
{
        int ret, retry = 0;
        fileid_t parentid, fid;
        char name[MAX_NAME_LEN], buf[MAX_BUF_LEN + 1];
        struct stat stbuf;
        uint64_t left, offset, size;
        lichbd_ioctx_t ioctx;

retry0:
        ret = stor_splitpath(path, &parentid, name);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_lookup(&parentid, name, &fid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_getattr(&fid, &stbuf);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

retry1:
        ret = lichbd_connect(path, &ioctx, 0);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry1, retry);
                } else
                        GOTO(err_ret, ret);
        }
        
        offset = 0;
        left = stbuf.st_size;
        while (left > 0) {
                size = left < MAX_BUF_LEN ? left : MAX_BUF_LEN;

        retry2:
                ret = lichbd_pread(&ioctx, buf, size, offset);
                if (ret < 0) {
                        ret = -ret;
                        if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry2, retry);
                        } else
                                GOTO(err_ret, ret);
                }
                
                YASSERT((uint64_t)ret == size);
                buf[size] = '\0';
                printf("%s", buf);
                fflush(stdout);

                offset += size;
                left -= size;
        }

        return 0;
err_ret:
        return ret;
}

int utils_write(const char *from, const char *to)
{
        int ret, retry = 0;
        fileid_t parentid, fid;
        char name[MAX_NAME_LEN];
        lichbd_ioctx_t ioctx;

retry3:
        ret = stor_splitpath(to, &parentid, name);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry3, retry);
                } else
                        GOTO(err_ret, ret);
        }

retry0:
        ret = stor_mkvol(&parentid, name, NULL, &fid);
        if (ret) {
                if (ret == EEXIST) {
                        ret = stor_lookup(&parentid, name, &fid);
                        if (ret)
                                GOTO(err_ret, ret);
                } else {
                        if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry0, retry);
                        } else
                                GOTO(err_ret, ret);
                }
        }

retry1:
        ret = lichbd_connect(to, &ioctx, O_CREAT);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry1, retry);
                } else
                        GOTO(err_ret, ret);
        }

retry2:
        ret = lichbd_pwrite(&ioctx, from, strlen(from) + 1, 0);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry2, retry);
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

int utils_fsstat(const char *path)
{
        int ret, retry = 0;
        fileid_t fid;
        struct statvfs svbuf;

retry0:
        ret = stor_lookup1(path, &fid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_statvfs(&fid, &svbuf);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        printf("bsize:%ld\nblocks:%ld\nbused:%ld\nbfree:%ld\n",
                        svbuf.f_bsize, svbuf.f_blocks, svbuf.f_blocks - svbuf.f_bavail, svbuf.f_bfree);
        return 0;
err_ret:
        return ret;
}

int __snapshot_copy_thread_create(arg_t * arg)
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

int __snapshot_copy_thread_startup(uint64_t chknum, arg_ext_t *arg_ext)
{
        int ret, count, i, err = 0;
        uint64_t left, offset, size, avg_size;
        arg_t args[COPY_THREAD_MAX];

        count = 0;
        offset = 0;
        left = chknum;
        avg_size = left % COPY_THREAD_MAX == 0 ? (left / COPY_THREAD_MAX) : (left / COPY_THREAD_MAX + 1);
        avg_size = avg_size < COPY_SIZE_MIN ? COPY_SIZE_MIN : avg_size;

        while (left > 0) {
                size = left < avg_size ? left : avg_size;
                args[count].offset = offset;
                args[count].size = size;
                args[count].ret = 0;
                args[count].ext = *arg_ext;

                ret = __snapshot_copy_thread_create(&args[count]);
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

int utils_snapshot(const char *name)
{
        int ret, count, retry = 0;
        char tmp[MAX_NAME_LEN];
        char *list[2];
        chkid_t chkid;

        count = 2;
        strcpy(tmp, name);
        _str_split(tmp, '@', list, &count); 
        if (count != 2) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        if (!is_valid_name(list[1], "snap")) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

retry:
        ret = stor_lookup1(list[0], &chkid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_snapshot_create(&chkid, list[1]);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }        

        return 0;
err_ret:
        return ret;
}

int utils_snapshot_rollback(const char *name)
{
        int ret, count, retry = 0;
        char tmp[MAX_NAME_LEN];
        char *list[2];
        chkid_t chkid;

        count = 2;
        strcpy(tmp, name);
        _str_split(tmp, '@', list, &count); 
        if (count != 2) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

retry:
        ret = stor_lookup1(list[0], &chkid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_snapshot_rollback(&chkid, list[1]);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }        

        return 0;
err_ret:
        return ret;
}

int utils_snapshot_remove(const char *name)
{
        int ret, count, retry = 0;
        char tmp[MAX_NAME_LEN];
        char *list[2];
        chkid_t chkid;

        count = 2;
        strcpy(tmp, name);
        _str_split(tmp, '@', list, &count); 
        if (count != 2) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

retry:
        ret = stor_lookup1(list[0], &chkid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_snapshot_remove(&chkid, list[1]);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }        

        return 0;
err_ret:
        return ret;
}

int utils_snapshot_src(const char *file, const char *snap, fileid_t *fileid,
                                       fileid_t *snapid, struct stat *stbuf)
{
        int ret, retry = 0;

retry0:
        ret = stor_lookup1(file, fileid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_lookup(fileid, snap, snapid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_getattr(snapid, stbuf);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

int utils_snapshot_clone__(const fileid_t *fileid, const fileid_t *snapid,
                                     struct stat *stbuf, const char *to)
{
        int ret, retry = 0;
        fileid_t parent, newfile;
        char name[MAX_NAME_LEN], src[MAX_NAME_LEN];
        setattr_t setattr;
        nid_t nid;

retry0:
        ret = stor_splitpath(to, &parent, name);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        memset(&setattr, 0x0, sizeof(setattr));
        setattr.size.set_it = 1;
        setattr.size.size = stbuf->st_size;
        setattr.clone.set_it = 1;
        setattr.clone.clone = 1;
        setattr.mode.set_it = 1;
        setattr.mode.mode = stbuf->st_mode;
        setattr.uid.set_it = 1;
        setattr.uid.uid = stbuf->st_uid;
        setattr.gid.set_it = 1;
        setattr.gid.gid = stbuf->st_gid;
        setattr.atime.set_it = __SET_TO_CLIENT_TIME;
        setattr.atime.time.seconds = stbuf->st_atime;
        setattr.mtime.set_it = __SET_TO_CLIENT_TIME;
        setattr.mtime.time.seconds = stbuf->st_mtime;

        ret = stor_mkvol(&parent, name, &setattr, &newfile);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

retry:
        ret = md_map_getsrv(&newfile, &nid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        snprintf(src, sizeof(src), CHKID_FORMAT"/"CHKID_FORMAT,
                 CHKID_ARG(fileid), CHKID_ARG(snapid));
        ret = md_xattr_set(&nid, &newfile, LICH_SYSTEM_ATTR_SOURCE,
                           src, strlen(src) + 1, O_EXCL);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

int utils_snapshot_clone(const char *name, const char *to)
{
        int ret, count, retry = 0;
        char tmp[MAX_NAME_LEN];
        char *list[2];
        fileid_t fileid, snapid;
        struct stat stbuf;

        count = 2;
        strcpy(tmp, name);
        _str_split(tmp, '@', list, &count); 
        if (count != 2) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

retry0:
        ret = utils_snapshot_src(list[0], list[1], &fileid, &snapid, &stbuf);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

retry1:
        ret = utils_snapshot_clone__(&fileid, &snapid, &stbuf, to);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry1, retry);
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

int utils_snapshot_protect(const char *name)
{
        int ret, count, retry = 0;
        char tmp[MAX_NAME_LEN];
        char *list[2];
        chkid_t chkid;

        count = 2;
        strcpy(tmp, name);
        _str_split(tmp, '@', list, &count); 
        if (count != 2) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

retry:
        ret = stor_lookup1(list[0], &chkid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_snapshot_protect(&chkid, list[1], 1);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }        

        return 0;
err_ret:
        return ret;
}

int utils_snapshot_unprotect(const char *name)
{
        int ret, count, retry = 0;
        char tmp[MAX_NAME_LEN];
        char *list[2];
        chkid_t chkid;

        count = 2;
        strcpy(tmp, name);
        _str_split(tmp, '@', list, &count); 
        if (count != 2) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

retry:
        ret = stor_lookup1(list[0], &chkid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = stor_snapshot_protect(&chkid, list[1], 0);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }        

        return 0;
err_ret:
        return ret;
}

int utils_snapshot_cat(const char *name)
{
        int ret, count, retry = 0;
        char tmp[MAX_NAME_LEN], buf[MAX_BUF_LEN];
        char *list[2];
        fileid_t parent, fileid;
        struct stat stbuf;
        lichbd_ioctx_t ioctx;

        count = 2;
        strcpy(tmp, name);
        _str_split(tmp, '@', list, &count); 
        if (count != 2) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

retry0:
        ret = utils_snapshot_src(list[0], list[1], &parent, &fileid, &stbuf);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }


        if (stbuf.st_size > MAX_BUF_LEN) {
                ret = EFBIG;
                GOTO(err_ret, ret);
        }

retry1:
        ret = lichbd_connect(list[0], &ioctx, 0);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry1, retry);
                } else
                        GOTO(err_ret, ret);
        }
        
retry2:
        ret = lichbd_snap_pread(&ioctx, &fileid, buf, stbuf.st_size, 0);
        if (ret < 0) {
                ret = -ret;
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry2, retry);
                } else
                        GOTO(err_ret, ret);
        }

        printf("%s\n", buf);

        return 0;
err_ret:
        return ret;
}

int utils_snapshot_copy_singlethreading(const fileid_t *fileid, const fileid_t *snapid,
                                     struct stat *stbuf, const char *to)
{
        int ret, fd, retry;
        char buf[BIG_BUF_LEN + 1];
        uint64_t left, offset, size;
        buffer_t buft;

        fd = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        mbuffer_init(&buft, 0);

        offset = 0;
        left = stbuf->st_size;
        while (left > 0) {
                size = left < BIG_BUF_LEN ? left : BIG_BUF_LEN;

                retry = 0;
        retry:
                ret = stor_snapshot_read(fileid, snapid, &buft, size, offset);
                if (ret) {
                        if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry, retry);
                        } else
                                GOTO(err_fd, ret);
                }

                YASSERT(buft.len == size);
                mbuffer_get(&buft, buf, buft.len);
                mbuffer_pop(&buft, NULL, buft.len);

                ret = pwrite(fd, buf, size, offset);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_fd, ret);
                }

                offset += size;
                left -= size;
        }

        close(fd);

        return 0;
err_fd:
        close(fd);
err_ret:
        return ret;
}

void * __snapshot_copy_worker(void *_arg)
{
        int ret, retry;
        arg_t *arg;
        char buf[BIG_BUF_LEN];
        buffer_t buft;
        uint64_t left, offset, size;

        mbuffer_init(&buft, 0);
        arg = _arg;

        offset = arg->offset;
        left = arg->size;
        while (left > 0) {
                size = left < BIG_BUF_LEN ? left : BIG_BUF_LEN;

                retry = 0;
        retry:
                ret = stor_snapshot_read(&arg->ext.fileid, &arg->ext.snapid, &buft, size, offset);
                if (ret) {
                        if (ret == EAGAIN) {
                                SLEEP_RETRY(err_ret, ret, retry, retry);
                        } else
                                GOTO(err_ret, ret);
                }

                YASSERT(buft.len == size);
                mbuffer_get(&buft, buf, buft.len);
                mbuffer_pop(&buft, NULL, buft.len);

                ret = pwrite(arg->ext.fd, buf, size, offset);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }

                offset += size;
                left -= size;
        }

        arg->ret = 0;
        sem_post(&arg->sem);

        return NULL;
err_ret:
        arg->ret = ret;
        sem_post(&arg->sem);
        return NULL;
}

int utils_snapshot_copy_multithreading(const fileid_t *fileid, const fileid_t *snapid,
                                     struct stat *stbuf, const char *to)
{
        int ret, fd;
        arg_ext_t arg_ext;

        fd = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        arg_ext.fd = fd;
        arg_ext.fileid = *fileid;
        arg_ext.snapid = *snapid;
        arg_ext.worker = __snapshot_copy_worker;

        ret = __snapshot_copy_thread_startup(stbuf->st_size, &arg_ext);
        if (ret)
                GOTO(err_fd, ret);


        close(fd);

        return 0;
err_fd:
        close(fd);
err_ret:
        return ret;
}

int utils_snapshot_copy(const char *name, const char *to)
{
        int ret, count, retry = 0;
        char tmp[MAX_NAME_LEN];
        char *list[2];
        fileid_t fileid, snapid;
        struct stat stbuf;

        (void)to;
        count = 2;
        strcpy(tmp, name);
        _str_split(tmp, '@', list, &count);
        if (count != 2) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

retry0:
        ret = utils_snapshot_src(list[0], list[1], &fileid, &snapid, &stbuf);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        if (name[0] != ':' && to[0] == ':') {
                if (MULTITHREADING) {
                        ret = utils_snapshot_copy_multithreading(&fileid, &snapid, &stbuf, to + 1);
                        if (ret)
                                GOTO(err_ret, ret);
                } else {
                        ret = utils_snapshot_copy_singlethreading(&fileid, &snapid, &stbuf, to + 1);
                        if (ret)
                                GOTO(err_ret, ret);
                }
        } else {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

/**
 * format:  0 -- lich
 *          1 -- json
 */
int utils_snapshot_list(const char *name, int all, int format)
{
        int ret, retry = 0;
        chkid_t chkid;
        char buf[BIG_BUF_LEN];
        int buflen = BIG_BUF_LEN;
        struct dirent *de;
        uint64_t offset;

retry:
        ret = stor_lookup1(name, &chkid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        if (chkid.type == __POOL_CHUNK__) {
                ret = EISDIR;
                GOTO(err_ret, ret);
        }

        ret = stor_snapshot_list(&chkid, buf, &buflen);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }        

        if (format == 1) {
                printf("[");
        }

        int begin = 1;
        dir_for_each(buf, buflen, de, offset) {
                if (strlen(de->d_name) == 0) {
                        break;
                }

                if (begin) {
                        begin = 0;
                } else {
                        //printf(",");
                }


                if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
                        continue;
                }

                if (!memcmp(de->d_name, LICH_SYSTEM_ATTR_UNLINK, strlen(LICH_SYSTEM_ATTR_UNLINK))) {
                        if (all)
                                printf("%s\n", de->d_name);
                } else {
                        if (format == 1) {
                                printf("{\"id\":4,\"name\":\"%s\",\"size\":10485760}", de->d_name);
                        } else {
                                printf("%s\n", de->d_name);
                        }
                }
        }
        if (format == 1) {
                printf("]");
        }

        return 0;
err_ret:
        return ret;
}
