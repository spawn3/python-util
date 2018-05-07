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
#include "lichbd.h"
#include "license.h"
#include "net_global.h"
#include "utils.h"
#include "dbg.h"

#define COPY_THREAD_MAX 10
#define COPY_SIZE_MIN   (32*1024*1024)  /* 32M */

typedef struct {
        int fd;
        lichbd_ioctx_t fioctx;
        lichbd_ioctx_t tioctx;
        void * (*worker)(void *_arg);
} arg_ext_t;

typedef struct {
        sem_t sem;
        int ret;
        arg_ext_t ext;
        off_t offset;
        size_t size;
} arg_t;

static int __copy_thread_create(arg_t * arg)
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

static int __copy_thread_startup(uint64_t _left, arg_ext_t *arg_ext)
{
        int ret, count, i, err = 0;
        uint64_t left, offset, size, avg_size;
        arg_t args[COPY_THREAD_MAX];

        count = 0;
        offset = 0;
        left = _left;
        avg_size = left % COPY_THREAD_MAX == 0 ? (left / COPY_THREAD_MAX) : (left / COPY_THREAD_MAX + 1);
        avg_size = avg_size < COPY_SIZE_MIN ? COPY_SIZE_MIN : avg_size;

        while (left > 0) {
                size = left < avg_size ? left : avg_size;
                args[count].offset = offset;
                args[count].size = size;
                args[count].ret = 0;
                args[count].ext = *arg_ext;

                ret = __copy_thread_create(&args[count]);
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

static int __stor_getattr(const char *path, fileid_t *fid, struct stat *stbuf)
{
        int ret, retry;

        retry = 0;
retry0:
        ret = stor_lookup1(path, fid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        retry = 0;
retry1:
        ret = stor_getattr(fid, stbuf);
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

static int __stor_create(const char *path, fileid_t *fid)
{
        int ret, retry;
        fileid_t parentid;
        char name[MAX_NAME_LEN];
        struct stat stbuf;

        retry = 0;
retry0:
        ret = stor_splitpath(path, &parentid, name);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else
                        GOTO(err_ret, ret);
        }

        retry = 0;
retry1:
        ret = stor_mkvol(&parentid, name, NULL, fid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry1, retry);
                } else if (ret == EEXIST) {
                        if (retry > 0) {
                                ret = stor_lookup(&parentid, name, fid);
                                if (ret)
                                        GOTO(err_ret, ret);

                                ret = stor_getattr(fid, &stbuf);
                                if (ret) {
                                        GOTO(err_ret, ret);
                                }

                                if (stbuf.st_size) {
                                        ret = EEXIST;
                                        GOTO(err_ret, ret);
                                }
                        } else
                                GOTO(err_ret, ret);
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __stor_truncate(const fileid_t *fileid, uint64_t size)
{
        int ret, retry = 0;
retry:
        ret = stor_truncate(fileid, size);
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

static int __stor_connect(const char *path, lichbd_ioctx_t *ioctx)
{
        int ret, retry = 0;
retry:
        ret = lichbd_connect(path, ioctx, 0);
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

static int __copy_from_stdout(const char *to)
{
        int ret, fd, retry;
        uint64_t offset, size, image_pos, buflen;
        char *buf;
        fileid_t fid;
        lichbd_ioctx_t ioctx;

        ret = __stor_create(to, &fid);
        if (ret)
                GOTO(err_ret, ret);

        ret = __stor_connect(to, &ioctx);
        if (ret)
                GOTO(err_ret, ret);

        buflen = LICH_IO_MAX * LICH_SPLIT_MAX;
        ret = ymalloc((void **)&buf, buflen);
        if (ret)
                GOTO(err_ret, ret);

        fd = 0;
        size = 0;
        offset = 0;
        image_pos = 0;
        while (1) {
                size = read(fd, buf + offset, buflen);
                if (ret < 0) {
                        ret = errno;
                        DERROR("read error, errno: %d\n", errno);
                        GOTO(err_free, ret);
                }

                offset += size;

                if (size && (size < buflen)) {
                        buflen -= size;
                        continue;
                }

                retry = 0;
        retry:
                ret = lichbd_pwrite(&ioctx, buf, offset, image_pos);
                if (ret) {
                        ret = _errno(ret);
                        if (ret == EAGAIN || ret == ENOSPC) {
                                USLEEP_RETRY(err_free, ret, retry, retry, 100, (100 * 1000));
                        } else
                                GOTO(err_free, ret);
                }

                image_pos += offset;

                if (size == 0) {
                        break;
                }

                offset = 0;
                buflen = LICH_IO_MAX * LICH_SPLIT_MAX;
        }

        yfree((void**)&buf);
        return 0;
err_free:
        yfree((void**)&buf);
err_ret:
        return ret;
}

static int __copy_from_local_singlethreading(const char *from, const char *to)
{
        int ret, fd, retry;
        struct stat stbuf;
        char buf[BIG_BUF_LEN];
        uint64_t left, offset, size;
        fileid_t fid;
        lichbd_ioctx_t ioctx;

        fd = open(from, O_RDONLY);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        ret = fstat(fd, &stbuf);
        if (ret)
                GOTO(err_fd, ret);

        ret = __stor_create(to, &fid);
        if (ret)
                GOTO(err_fd, ret);

        ret = __stor_truncate(&fid, stbuf.st_size);
        if (ret)
                GOTO(err_fd, ret);

        ret = __stor_connect(to, &ioctx);
        if (ret)
                GOTO(err_fd, ret);

        offset = 0;
        left = stbuf.st_size;
        while (left > 0) {
                size = left < BIG_BUF_LEN ? left : BIG_BUF_LEN;

                ret = pread(fd, buf, size, offset);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_fd, ret);
                }

                retry = 0;
        retry:
                ret = lichbd_pwrite(&ioctx, buf, size, offset);
                if (ret) {
                        ret = _errno(ret);
                        if (ret == EAGAIN || ret == ENOSPC) {
                                USLEEP_RETRY(err_fd, ret, retry, retry, 100, (100 * 1000));
                        } else
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

static void * __copy_from_local_worker(void *_arg)
{
        int ret, retry;
        arg_t *arg;
        char buf[BIG_BUF_LEN];
        uint64_t left, offset, size;

        arg = _arg;

        offset = arg->offset;
        left = arg->size;
        while (left > 0) {
                size = left < BIG_BUF_LEN ? left : BIG_BUF_LEN;

                ret = pread(arg->ext.fd, buf, size, offset);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }

                retry = 0;
        retry:
                ret = lichbd_pwrite(&arg->ext.tioctx, buf, size, offset);
                if (ret) {
                        ret = _errno(ret);
                        if (ret == EAGAIN || ret == ENOSPC) {
                                USLEEP_RETRY(err_ret, ret, retry, retry, 100, (100 * 1000));
                        } else
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

static int __copy_from_local_multithreading(const char *from, const char *to)
{
        int ret, fd;
        struct stat stbuf;
        fileid_t fid;
        arg_ext_t arg_ext;
        lichbd_ioctx_t ioctx;

        fd = open(from, O_RDONLY);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        ret = fstat(fd, &stbuf);
        if (ret)
                GOTO(err_fd, ret);

        ret = __stor_create(to, &fid);
        if (ret)
                GOTO(err_fd, ret);

        ret = __stor_truncate(&fid, stbuf.st_size);
        if (ret)
                GOTO(err_fd, ret);

        ret = __stor_connect(to, &ioctx);
        if (ret)
                GOTO(err_ret, ret);

        arg_ext.fd = fd;
        arg_ext.tioctx = ioctx;
        arg_ext.worker = __copy_from_local_worker;

        ret = __copy_thread_startup(stbuf.st_size, &arg_ext);
        if (ret)
                GOTO(err_fd, ret);

        close(fd);

        return 0;
err_fd:
        close(fd);
err_ret:
        return ret;
}

static int __copy_to_local_singlethreading(const char *from, const char *to)
{
        int ret, fd, retry = 0;
        char buf[BIG_BUF_LEN + 1];
        struct stat stbuf;
        uint64_t left, offset, size;
        fileid_t fid;
        lichbd_ioctx_t ioctx;

        ret = __stor_getattr(from, &fid, &stbuf);
        if (ret)
                GOTO(err_ret, ret);

        ret = __stor_connect(from, &ioctx);
        if (ret)
                GOTO(err_ret, ret);

        fd = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        offset = 0;
        left = stbuf.st_size;
        while (left > 0) {
                size = left < BIG_BUF_LEN ? left : BIG_BUF_LEN;

        retry:
                ret = lichbd_pread(&ioctx, buf, size, offset);
                if (ret < 0) {
                        ret = -ret;
                        if (ret == EAGAIN) {
                                USLEEP_RETRY(err_ret, ret, retry, retry, 100, (100 * 1000));
                        } else
                                GOTO(err_fd, ret);
                }

                YASSERT((uint64_t)ret == size);
                ret = pwrite(fd, buf, size, offset);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_fd, ret);
                }

                offset += size;
                left -= size;
        }

        return 0;
err_fd:
        close(fd);
err_ret:
        return ret;
}

static void * __copy_to_local_worker(void *_arg)
{
        int ret, retry;
        arg_t *arg;
        char buf[BIG_BUF_LEN];
        uint64_t left, offset, size;

        arg = _arg;

        offset = arg->offset;
        left = arg->size;
        while (left > 0) {
                size = left < BIG_BUF_LEN ? left : BIG_BUF_LEN;

                retry = 0;
        retry:
                ret = lichbd_pread(&arg->ext.fioctx, buf, size, offset);
                if (ret < 0) {
                        ret = -ret;
                        if (ret == EAGAIN) {
                                USLEEP_RETRY(err_ret, ret, retry, retry, 100, (100 * 1000));
                        } else
                                GOTO(err_ret, ret);
                }

                YASSERT((uint64_t)ret == size);
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

static int __copy_to_local_multithreading(const char *from, const char *to)
{
        int ret, fd;
        struct stat stbuf;
        arg_ext_t arg_ext;
        fileid_t fid;
        lichbd_ioctx_t ioctx;

        ret = __stor_getattr(from, &fid, &stbuf);
        if (ret)
                GOTO(err_ret, ret);

        ret = __stor_connect(from, &ioctx);
        if (ret)
                GOTO(err_ret, ret);

        fd = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        arg_ext.fd = fd;
        arg_ext.fioctx = ioctx;
        arg_ext.worker = __copy_to_local_worker;

        ret = __copy_thread_startup(stbuf.st_size, &arg_ext);
        if (ret)
                GOTO(err_fd, ret);

        close(fd);

        return 0;
err_fd:
        close(fd);
err_ret:
        return ret;
}

static int __copy_standard_singlethreading(const char *from, const char *to)
{
        int ret, retry = 0;
        struct stat stbuf;
        char buf[BIG_BUF_LEN];
        uint64_t left, offset, size;
        fileid_t fid, src;
        lichbd_ioctx_t fioctx, tioctx;

        ret = __stor_getattr(from, &src, &stbuf);
        if (ret)
                GOTO(err_ret, ret);

        ret = __stor_connect(from, &fioctx);
        if (ret)
                GOTO(err_ret, ret);

        ret = __stor_create(to, &fid);
        if (ret)
                GOTO(err_ret, ret);

        ret = __stor_connect(to, &tioctx);
        if (ret)
                GOTO(err_ret, ret);

        offset = 0;
        left = stbuf.st_size;
        while (left > 0) {
                size = left < BIG_BUF_LEN ? left : BIG_BUF_LEN;

                retry = 0;
        retry1:
                ret = lichbd_pread(&fioctx, buf, size, offset);
                if (ret < 0) {
                        ret = -ret;
                        if (ret == EAGAIN) {
                                USLEEP_RETRY(err_ret, ret, retry1, retry, 100, (100 * 1000));
                        } else
                                GOTO(err_ret, ret);
                }

                retry = 0;
        retry2:
                ret = lichbd_pwrite(&tioctx, buf, size, offset);
                if (ret) {
                        ret = _errno(ret);
                        if (ret == EAGAIN || ret == ENOSPC) {
                                USLEEP_RETRY(err_ret, ret, retry2, retry, 100, (100 * 1000));
                        } else
                                GOTO(err_ret, ret);
                }

                offset += size;
                left -= size;
        }

        return 0;
err_ret:
        return ret;
}

static void * __copy_standard_worker(void *_arg)
{
        int ret, retry;
        arg_t *arg;
        char buf[BIG_BUF_LEN];
        uint64_t left, offset, size;

        arg = _arg;

        offset = arg->offset;
        left = arg->size;
        while (left > 0) {
                size = left < BIG_BUF_LEN ? left : BIG_BUF_LEN;

                retry = 0;
        retry1:
                ret = lichbd_pread(&arg->ext.fioctx, buf, size, offset);
                if (ret < 0) {
                        ret = -ret;
                        if (ret == EAGAIN) {
                                USLEEP_RETRY(err_ret, ret, retry1, retry, 100, (100 * 1000));
                        } else
                                GOTO(err_ret, ret);
                }

                retry = 0;
        retry2:
                ret = lichbd_pwrite(&arg->ext.tioctx, buf, size, offset);
                if (ret) {
                        if (ret == EAGAIN) {
                                USLEEP_RETRY(err_ret, ret, retry2, retry, 100, (100 * 1000));
                        } else
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

static int __copy_standard_multithreading(const char *from, const char *to)
{
        int ret;
        struct stat stbuf;
        fileid_t fid, src;
        arg_ext_t arg_ext;
        lichbd_ioctx_t fioctx, tioctx;

        ret = __stor_getattr(from, &src, &stbuf);
        if (ret)
                GOTO(err_ret, ret);

        ret = __stor_connect(from, &fioctx);
        if (ret)
                GOTO(err_ret, ret);

        ret = __stor_create(to, &fid);
        if (ret)
                GOTO(err_ret, ret);

        ret = __stor_connect(to, &tioctx);
        if (ret)
                GOTO(err_ret, ret);

        arg_ext.fioctx = fioctx;
        arg_ext.tioctx = tioctx;
        arg_ext.worker = __copy_standard_worker;

        ret = __copy_thread_startup(stbuf.st_size, &arg_ext);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

int stor_copy(const char *from, const char *to, int multithreading)
{
        printf("copy from %s to %s\n", from, to);

        if (from[0] == ':' && to[0] != ':') {
                if (from[1] == '-')
                        return __copy_from_stdout(to);
                else if (multithreading)
                        return __copy_from_local_multithreading(from + 1, to);
                else
                        return __copy_from_local_singlethreading(from + 1, to);
        } else if (from[0] != ':' && to[0] == ':') {
                if (multithreading)
                        return __copy_to_local_multithreading(from, to + 1);
                else
                        return __copy_to_local_singlethreading(from, to + 1);
        } else if (from[0] != ':' && to[0] != ':') {
                if (multithreading)
                        return __copy_standard_multithreading(from, to);
                else
                        return __copy_standard_singlethreading(from, to);
        } else {
                UNIMPLEMENTED(__DUMP__);
        }

        return 0;
}
