#include "config.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <dirent.h>
#include <ctype.h>

#include "configure.h"
#include "cluster.h"
#include "storage.h"
#include "utils.h"
#include "yiscsi.h"

#define CLONE_JOB_MAX   (128)
#define CLONE_BUF_MAX   (1024 * 1024)
//#define CLONE_BUF_MAX   (1000*1000)

typedef struct {
        buffer_t buf;
        uint64_t off;
        int retry;
        int size;
        int replace;
        union {
                fileid_t oid;
                int   fd;
                char *path;
        } from, to;
        sem_t *sem;
} clone_ctx_t;

static worker_handler_t clone_jobtracker;

const char __zero_buf[CLONE_BUF_MAX] = { 0 };

static int __stm_local2lun(job_t *job)
{
        int ret;
        clone_ctx_t *context = job->context;

        ret = job_get_ret(job, 0);
        if (ret) {
                if (object_errno(ret) == EAGAIN && context->retry < 30) {
                        if (context->retry > 3)
                                DWARN("write @ %llu retry %u\n", (LLU)context->off, context->retry);
                        context->retry++;
                        sleep(1);
                        ret = object_write(&context->to.oid, &context->buf,
                                           context->size, context->off, job_handler(job, 0));
                        if (ret) {
                                if (context->replace == 0)
                                        object_removebyid(&context->to.oid);
                                DERROR("error %d(%s)\n", ret, strerror(ret));
                                EXIT(ret);
                        }

                        return 0;
                } else {
                        DERROR("error %d(%s)\n", ret, strerror(ret));
                        EXIT(ret);
                }
        }

        sem_post(context->sem);
        mbuffer_free(&context->buf);
        job_destroy(job);

        return 0;
}

int clone_local2lun(const char *from, const char *to, int replace)
{
        int ret, fd, cnt, retry, tgt_len;
        char buf[CLONE_BUF_MAX];
        fileid_t oid;
        uint64_t rest, _rest, off;
        struct stat stbuf;
        clone_ctx_t *context;
        job_t *job;
        sem_t wait_sem;

        fd = open(from, O_RDONLY);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        tgt_len = get_tgt_len(to);
        /*         iqn            :          name(namespace . target) */
        if (strlen(sanconf.iqn) + 1 + tgt_len > ISCSI_TGT_NAME_MAX) {
                fprintf(stderr, "\x1b[1;31minvalid name length, namespace and target maximum length is %ld\x1b[0m\n",
                        ISCSI_TGT_NAME_MAX - strlen(sanconf.iqn) - 1 - 1);
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        if (!is_valid_name(to, 0)) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        ret = object_create(to, &oid);
        if (ret) {
                if (ret == EEXIST && replace) {
                        ret = object_open(to, &oid);
                        if (ret)
                                GOTO(err_fd, ret);
                } else
                        GOTO(err_fd, ret);
        }

        ret = fstat(fd, &stbuf);
        if (ret < 0) {
                ret = errno;
                GOTO(err_obj, ret);
        }

        off = 0;
        rest = _rest = stbuf.st_size;

        ret = sem_init(&wait_sem, 0, CLONE_JOB_MAX);
        if (ret)
                GOTO(err_obj, ret);

        while (rest > 0) {
                if (rest >= CLONE_BUF_MAX)
                        cnt = CLONE_BUF_MAX;
                else
                        cnt = (int)rest;

                ret = pread(fd, buf, cnt, off);
                if (ret != cnt) {
                        ret = EIO;
                        GOTO(err_sem, ret);
                }

                /*
                 * If this is a zero buffer, skip it
                 */
                if (!memcmp(__zero_buf, buf, cnt)) {
                        DBUG("skip zero buffer\n");
                        goto next;
                }

                ret = job_create(&job, &clone_jobtracker, "clone_local2lun");
                if (ret)
                        GOTO(err_sem, ret);

                ret = job_context_create(job, sizeof(clone_ctx_t));
                if (ret)
                        GOTO(err_job, ret);

                context = job->context;

                ret = mbuffer_init(&context->buf, 0);
                if (ret)
                        GOTO(err_job, ret);

                ret = mbuffer_copy(&context->buf, buf, cnt);
                if (ret)
                        GOTO(err_job, ret);

                context->to.oid    = oid;
                context->sem       = &wait_sem;
                context->off = off;
                context->size = cnt;
                context->retry = 0;
                context->replace = replace;
                job->state_machine = __stm_local2lun;

                retry = 0;
        retry:
                ret = object_write(&context->to.oid, &context->buf,
                                   context->size, context->off, job_handler(job, 0));
                if (ret) {
                        if (ret == EAGAIN) {
                                sleep(1);
                                SLEEP_RETRY(err_job, ret, retry, retry);
                        } else
                                GOTO(err_job, ret);
                }

                sem_wait(&wait_sem);
next:
                off += cnt;
                rest -= cnt;
        }

        while (1) {
                int val;

                sem_getvalue(&wait_sem, &val);

                if (val == CLONE_JOB_MAX)
                        break;

                sleep(1);
        }

        /* Truncate lun to fix length, then iscsi server can find it */
        ret = object_truncate(&oid, _rest);
        if (ret)
                GOTO(err_sem, ret);

        sem_destroy(&wait_sem);
        close(fd);

        return 0;
err_job:
        job_destroy(job);
err_sem:
        sem_destroy(&wait_sem);
err_obj:
        if (replace == 0)
                object_remove(to, 0);
err_fd:
        close(fd);
err_ret:
        return object_errno(ret);
}

static int __stm_lun2lun(job_t *job)
{
        int ret, retry = 0;
        char buf[CLONE_BUF_MAX];
        clone_ctx_t *context = job->context;

        ret = job_get_ret(job, 0);
        if (ret) {
                object_removebyid(&context->to.oid);
                DERROR("error %d(%s)\n", ret, strerror(ret));
                EXIT(ret);
        }

        ret = mbuffer_get(&context->buf, buf, context->size);
        if (ret) {
                DERROR("error %d(%s)\n", ret, strerror(ret));
                EXIT(EIO);
        }

#if 1
        if (!memcmp(__zero_buf, buf, context->size)) {
                DBUG("skip zero buffer\n");
                goto out;
        }
#endif

retry:
        ret = object_pwrite(&context->to.oid, buf, context->size, context->off);
        if (ret) {
                if (ret == EAGAIN || ret == ENOENT) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else {
                        DERROR("error %d(%s)\n", ret, strerror(ret));
                        EXIT(EAGAIN);
                }
        }

out:
        sem_post(context->sem);
        mbuffer_free(&context->buf);
        job_destroy(job);
        return 0;
err_ret:
        DERROR("error %d(%s)\n", ret, strerror(ret));
        EXIT(EAGAIN);
        return ret;
}

int clone_lun2lun(const char *from, const char *to)
{
        int ret, cnt;
        char _buf[MAX_BUF_LEN];
        fileid_t oid_from, fileid_to;
        oinfo_t *oinfo;
        uint64_t rest, _rest, off;
        clone_ctx_t *context;
        job_t *job;
        sem_t wait_sem;

        ret = object_open(from, &oid_from);
        if (ret)
                GOTO(err_ret, ret);

        ret = object_create(to, &fileid_to);
        if (ret)
                GOTO(err_ret, ret);

        oinfo = (void *)_buf;
        ret = object_getattr(&oid_from, oinfo, 1);
        if (ret)
                GOTO(err_obj, ret);

        off  = 0;
        rest = _rest = oinfo->size;

        ret = sem_init(&wait_sem, 0, CLONE_JOB_MAX);
        if (ret)
                GOTO(err_obj, ret);

        while (rest > 0) {
                if (rest >= CLONE_BUF_MAX)
                        cnt = CLONE_BUF_MAX;
                else
                        cnt = (int)rest;

                ret = job_create(&job, &clone_jobtracker, "clone_lun2lun");
                if (ret)
                        GOTO(err_sem, ret);

                ret = job_context_create(job, sizeof(clone_ctx_t));
                if (ret)
                        GOTO(err_job, ret);

                context = job->context;

                ret = mbuffer_init(&context->buf, 0);
                if (ret)
                        GOTO(err_job, ret);

                context->off       = off;
                context->size      = cnt;
                context->to.oid    = fileid_to;
                context->sem       = &wait_sem;
                job->state_machine = __stm_lun2lun;

                ret = object_read(&oid_from, &context->buf, cnt, off, job_handler(job, 0));
                if (ret)
                        GOTO(err_job, ret);

                sem_wait(&wait_sem);

                off += cnt;
                rest -= cnt;
        }

        while (1) {
                int val;

                sem_getvalue(&wait_sem, &val);

                if (val == CLONE_JOB_MAX)
                        break;

                sleep(1);
        }

        /* Truncate lun to fix length, then iscsi server can find it */
        ret = object_truncate(&fileid_to, _rest);
        if (ret)
                GOTO(err_sem, ret);

        sem_destroy(&wait_sem);

        return 0;
err_job:
        job_destroy(job);
err_sem:
        sem_destroy(&wait_sem);
err_obj:
        object_remove(to, 0);
err_ret:
        return object_errno(ret);
}

static int __stm_lun2local(job_t *job)
{
        int ret;
        char buf[CLONE_BUF_MAX];
        clone_ctx_t *context = job->context;

        ret = job_get_ret(job, 0);
        if (ret) {
                DWARN("error %d(%s)\n", ret, strerror(ret));
                EXIT(ret);
        }

        ret = mbuffer_get(&context->buf, buf, context->size);
        if (ret) {
                DERROR("error %d(%s)\n", ret, strerror(ret));
                EXIT(EIO);
        }

        if (!memcmp(__zero_buf, buf, context->size)) {
                DBUG("skip zero buffer\n");
                goto out;
        }

        ret = pwrite(context->to.fd, buf, context->size, context->off);
        if (ret != context->size) {
                DERROR("error %d(%s)\n", ret, strerror(ret));
                EXIT(EAGAIN);
        }

out:
        sem_post(context->sem);
        mbuffer_free(&context->buf);
        job_destroy(job);

        return 0;
}

int clone_lun2local(const char *from, const char *to)
{
        int ret, fd, cnt, retry;
        char buf[MAX_BUF_LEN];
        fileid_t oid;
        oinfo_t *oinfo;
        uint64_t rest, _rest, off;
        clone_ctx_t *context;
        job_t *job;
        sem_t wait_sem;

        retry = 0;
retry1:
        ret = object_open(from, &oid);
        if (ret) {
                if (ret == EAGAIN || ret == ENOENT) {
                        SLEEP_RETRY2(err_ret, ret, retry1, retry, 30);
                        GOTO(err_ret, ret);
                } else
                        GOTO(err_ret, ret);
        }

        fd = open(to, O_CREAT | O_WRONLY | O_EXCL, 0644);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        oinfo = (void *)buf;
        ret = object_getattr(&oid, oinfo, 1);
        if (ret)
                GOTO(err_fd, ret);

        off  = 0;
        rest = _rest = oinfo->size;

        ret = sem_init(&wait_sem, 0, CLONE_JOB_MAX);
        if (ret)
                GOTO(err_fd, ret);

        while (rest > 0) {
                if (rest >= CLONE_BUF_MAX)
                        cnt = CLONE_BUF_MAX;
                else
                        cnt = (int)rest;

                ret = job_create(&job, &clone_jobtracker, "clone_lun2local");
                if (ret)
                        GOTO(err_sem, ret);

                ret = job_context_create(job, sizeof(clone_ctx_t));
                if (ret)
                        GOTO(err_job, ret);

                context = job->context;

                ret = mbuffer_init(&context->buf, 0);
                if (ret)
                        GOTO(err_job, ret);

                context->off       = off;
                context->size      = cnt;
                context->to.fd     = fd;
                context->sem       = &wait_sem;
                job->state_machine = __stm_lun2local;

                retry = 0;
        retry2:
                ret = object_read(&oid, &context->buf, cnt, off, job_handler(job, 0));
                if (ret) {
                        if (ret == EAGAIN) {
                                SLEEP_RETRY2(err_job, ret, retry2, retry, 30);
                        }
                        GOTO(err_job, ret);
                }

                sem_wait(&wait_sem);

                off += cnt;
                rest -= cnt;
        }

        while (1) {
                int val;

                sem_getvalue(&wait_sem, &val);

                if (val == CLONE_JOB_MAX)
                        break;

                sleep(1);
        }

#if 1
        ret = ftruncate(fd, _rest);
        if (ret) {
                ret = errno;
                GOTO(err_sem, ret);
        }
#endif

        sem_destroy(&wait_sem);
        close(fd);

        return 0;
err_job:
        job_destroy(job);
err_sem:
        sem_destroy(&wait_sem);
err_fd:
        close(fd);
err_ret:
        return object_errno(ret);
}

int append_local2lun(const char *from, const char *to)
{
        int ret, fd, cnt, retry = 0, tgt_len;
        char buf[CLONE_BUF_MAX], tmp[MAX_BUF_LEN];
        fileid_t oid;
        uint64_t rest, off, woff;
        struct stat stbuf;
        oinfo_t *oinfo;

        fd = open(from, O_RDONLY);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        tgt_len = get_tgt_len(to);
        /*         iqn            :          name(namespace . target) */
        if (strlen(sanconf.iqn) + 1 + tgt_len > ISCSI_TGT_NAME_MAX) {
                fprintf(stderr, "\x1b[1;31minvalid name length, namespace and target maximum length is %ld\x1b[0m\n",
                        ISCSI_TGT_NAME_MAX - strlen(sanconf.iqn) - 1 - 1);
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        if (!is_valid_name(to, 0)) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        ret = object_open(to, &oid);
        if (ret) {
                if (ret == ENOENT) {
                        ret = object_create(to, &oid);
                        if (ret)
                                GOTO(err_fd, ret);
                } else
                        GOTO(err_fd, ret);
        }

        oinfo = (void *)tmp;
        ret = object_getattr(&oid, oinfo, 1);
        if (ret)
                GOTO(err_fd, ret);

        woff = oinfo->size;

        ret = fstat(fd, &stbuf);
        if (ret < 0) {
                ret = errno;
                GOTO(err_fd, ret);
        }

        off = 0;
        rest = stbuf.st_size;

        while (rest > 0) {
                if (rest >= CLONE_BUF_MAX)
                        cnt = CLONE_BUF_MAX;
                else
                        cnt = (int)rest;

                ret = pread(fd, buf, cnt, off);
                if (ret != cnt) {
                        ret = EIO;
                        GOTO(err_fd, ret);
                }

                if (memcmp(__zero_buf, buf, cnt)) {
                retry1:
                        ret = object_pwrite(&oid, buf, cnt, woff + off);
                        if (ret) {
                                if (ret == EAGAIN) {
                                        SLEEP_RETRY1(err_fd, EIO, retry1, retry);
                                } else {
                                        DERROR("error %d(%s)\n", ret, strerror(ret));
                                        EXIT(EAGAIN);
                                }
                        }
                } else {
                retry2:
                        ret = object_truncate(&oid, cnt + woff + off);
                        if (ret) {
                                if (ret == EAGAIN) {
                                        SLEEP_RETRY1(err_fd, EIO, retry2, retry);
                                } else {
                                        DERROR("error %d(%s)\n", ret, strerror(ret));
                                        EXIT(EAGAIN);
                                }
                        }
                }

                off += cnt;
                rest -= cnt;
        }

        close(fd);

        return 0;
err_fd:
        close(fd);
err_ret:
        return ret;
}

int clone_init(int thread)
{
        int ret;

        (void) thread;

        ret = jobdock_worker_create(&clone_jobtracker, "clone");
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

int clone_lun2lun1(const char *from, const char *to)
{
        int ret, cnt, retry = 0, tgt_len;
        char _buf[MAX_BUF_LEN], *buf;
        fileid_t oid_from, fileid_to;
        oinfo_t *oinfo;
        uint64_t rest, _rest, off;

        ret = object_open(from, &oid_from);
        if (ret)
                GOTO(err_ret, ret);

        tgt_len = get_tgt_len(to);
        /*         iqn            :          name(namespace . target) */
        if (strlen(sanconf.iqn) + 1 + tgt_len > ISCSI_TGT_NAME_MAX) {
                fprintf(stderr, "\x1b[1;31minvalid name length, namespace and target maximum length is %ld\x1b[0m\n",
                        ISCSI_TGT_NAME_MAX - strlen(sanconf.iqn) - 1 - 1);
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        if (!is_valid_name(to, 0)) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        ret = object_create(to, &fileid_to);
        if (ret)
                GOTO(err_ret, ret);

        oinfo = (void *)_buf;
        ret = object_getattr(&oid_from, oinfo, 1);
        if (ret)
                GOTO(err_obj, ret);

        off  = 0;
        rest = _rest = oinfo->size;

        ret = ymalloc((void **)&buf, CLONE_BUF_MAX);
        if (ret)
                GOTO(err_obj, ret);

        while (rest > 0) {
                if (rest >= CLONE_BUF_MAX)
                        cnt = CLONE_BUF_MAX;
                else
                        cnt = (int)rest;

                retry = 0;
        retry1:
                ret = object_pread(&oid_from, buf, cnt, off);
                if (ret < 0) {
                        ret = -ret;
                        if (ret == EAGAIN) {
                                SLEEP_RETRY1(err_free, ret, retry1, retry);
                        } else
                                GOTO(err_free, ret);
                }

                if (memcmp(__zero_buf, buf, cnt)) {
                        retry = 0;
                retry2:
                        ret = object_pwrite(&fileid_to, buf, cnt, off);
                        if (ret) {
                                if (ret == EAGAIN) {
                                        SLEEP_RETRY1(err_free, ret, retry2, retry);
                                } else
                                        GOTO(err_free, ret);
                        }
                } else {
                        DBUG("skip zero buffer\n");
                }

                off += cnt;
                rest -= cnt;
        }

                /* Truncate lun to fix length, then iscsi server can find it */
        ret = object_truncate(&fileid_to, _rest);
        if (ret)
                GOTO(err_free, ret);

        yfree((void **)&buf);

        return 0;
err_free:
        yfree((void **)&buf);
err_obj:
        object_remove(to, 0);
err_ret:
        return ret;
}
