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
#include "../../storage/storage/md_parent.h"
#include "../../storage/storage/stor_rpc.h"
#include "lichstor.h"
#include "prof.h"
#include "license.h"
#include "utils.h"
#include "net_global.h"
#include "dbg.h"
#include "chunk.h"

typedef enum {
        OP_NULL,
        OP_RPC,
        OP_NET,
        OP_VM,
        OP_DIO,
        OP_LICHBD,
        //OP_DIRECT,
        OP_SCHEDULE,
} admin_op_t;


static void usage()
{
        fprintf(stderr, "\nusage:\n"
                "lich.prof --rpc <node> [--sendsize size] [--recvsize size] [--thread num] [--runtime num]\n"
                "lich.prof --net <node> [--sendsize size] [--recvsize size] [--thread num] [--runtime num]\n"
                "lich.prof --vm <node> [--sendsize size] [--recvsize size] [--thread num] [--runtime num]\n"
                "lich.prof --dio <node> [--sendsize size] [--recvsize size] [--thread num] [--runtime num]\n"
                "lich.prof --lichbd  r | w [--size size] [--thread num] [--runtime num] [--volume vol1,vol2,vol3] \n"
                "lich.prof --schedule\n"
                );
}

typedef struct {
        sem_t sem;
        uint64_t io;
        nid_t nid;
        int fd;
        int sendsize;
        int recvsize;
} prof_net_arg_t;

static void *__prof_net_worker(void *_arg)
{
        int ret;
        prof_net_arg_t *arg = _arg;

        while (1) {
                ret = prof_rpc_net(&arg->nid, arg->sendsize, arg->recvsize);
                if (ret)
                        UNIMPLEMENTED(__DUMP__);

                arg->io++;

        }

        return NULL;
}

static void __lich_net_print(prof_net_arg_t *args, int threads, struct timeval *begin)
{
        int i;
        uint64_t io, used;
        struct timeval now;

        io = 0;
        for (i = 0; i < threads; i++) {
                io += args[i].io;
        }

        _gettimeofday(&now, NULL);

        used = _time_used(begin, &now);

        DINFO("io %u, used %u\n", (int)io, (int)used);
        printf("iops : %llu\n", (long long)io * 1000 * 1000 /  used);
}

static int __lich_rpc(const char *nodename, const char *_sendsize, const char *_recvsize,
                      const char *_threads, const char *_runtime)
{
        int ret, sendsize, recvsize, threads, runtime, i;
        nid_t nid;
        struct timeval begin;
        prof_net_arg_t args[1024], *arg;
        pthread_t th;
        pthread_attr_t ta;

        ret = env_init_simple("lich.prof");
        if (ret)
                GOTO(err_ret, ret);

        ret = network_connect_master();
        if (ret) {
                GOTO(err_ret, ret);
        }

        ret = stor_init(NULL, -1);
        if (ret)
                GOTO(err_ret, ret);

        ret = prof_rpc_init();
        if (ret)
                GOTO(err_ret, ret);

        sendsize = atoi(_sendsize);
        recvsize = atoi(_recvsize);
        threads = atoi(_threads);
        if (_runtime == NULL)
                runtime = INT32_MAX;
        else
                runtime = atoi(_runtime);

        YASSERT(threads < 1024);

        ret = network_connect_byname(nodename, &nid);
        if (ret)
                GOTO(err_ret, ret);

        (void) pthread_attr_init(&ta);
        (void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);

        _gettimeofday(&begin, NULL);

        for (i = 0; i < threads; i++) {
                arg = &args[i];
                ret = sem_init(&arg->sem, 0, 0);
                if (ret)
                        UNIMPLEMENTED(__DUMP__);

                arg->io = 0;
                arg->sendsize = sendsize;
                arg->recvsize = recvsize;
                arg->nid = nid;

                ret = pthread_create(&th, &ta, __prof_net_worker, arg);
                if (ret)
                        GOTO(err_ret, ret);
        }

        //end = gettime();
        i = 0;
        while (i < runtime) {
                sleep(1);
                i++;
                __lich_net_print(args, threads, &begin);
        }

        return 0;
err_ret:
        return ret;
}


static int __lich_connect_byname(const char *name, int *sd)
{
        int ret;
        char buf[MAX_BUF_LEN];
        ynet_net_info_t *info;
        net_handle_t nh;
        nid_t nid;
        uint32_t infolen;

        ret = maping_host2nid(name, &nid);
        if (ret) {
                DWARN("%s ret (%u) %s\n", name, ret, strerror(ret));
                GOTO(err_ret, ret);
        }

        info = (void *)buf;
        ret = maping_nid2netinfo(&nid, info);
        if (ret)
                GOTO(err_ret, ret);

        ret = sock_info2sock(&nh, &info->info[0], 0, 10);
        if (ret) {
                GOTO(err_ret, ret);
        }

        infolen = MAX_BUF_LEN;
        ret = net_getinfo(buf, &infolen, ng.port);
        if (ret) {
                GOTO(err_ret, ret);
        }

        ret = _send(nh.u.sd.sd, (void *)buf, infolen,
                    MSG_NOSIGNAL | MSG_DONTWAIT);
        if (ret < 0) {
                ret = -ret;
                GOTO(err_ret, ret);
        } else if ((uint32_t)ret != infolen) {
                ret = EBADF;
                DWARN("bad sd %u\n", nh.u.sd.sd);
                GOTO(err_ret, ret);
        }

        *sd = nh.u.sd.sd;

        return 0;
err_ret:
        return ret;
}

static void *__prof_direct_worker(void *_arg)
{
        int ret;
        prof_net_arg_t *arg = _arg;

        while (1) {
                ret = prof_rpc_direct(arg->fd, arg->sendsize, arg->recvsize);
                if (ret)
                        UNIMPLEMENTED(__DUMP__);

                arg->io++;

        }

        return NULL;
}

static int __lich_rpc_direct(const char *nodename, const char *_sendsize, const char *_recvsize,
                      const char *_threads, const char *_runtime)
{
        int ret, sendsize, recvsize, threads, runtime, i, fd;
        nid_t nid = {0};
        struct timeval begin;
        prof_net_arg_t args[1024], *arg;
        pthread_t th;
        pthread_attr_t ta;

        sendsize = atoi(_sendsize);
        recvsize = atoi(_recvsize);
        threads = atoi(_threads);
        if (_runtime == NULL)
                runtime = INT32_MAX;
        else
                runtime = atoi(_runtime);

        YASSERT(threads < 1024);

        (void) pthread_attr_init(&ta);
        (void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);

        _gettimeofday(&begin, NULL);

        for (i = 0; i < threads; i++) {
                arg = &args[i];
                ret = sem_init(&arg->sem, 0, 0);
                if (ret)
                        UNIMPLEMENTED(__DUMP__);

                arg->io = 0;
                arg->sendsize = sendsize;
                arg->recvsize = recvsize;
                arg->nid = nid;

                ret = __lich_connect_byname(nodename, &fd);
                if (ret)
                        GOTO(err_ret, ret);

                arg->fd = fd;

                ret = pthread_create(&th, &ta, __prof_direct_worker, arg);
                if (ret)
                        GOTO(err_ret, ret);
        }

        //end = gettime();
        i = 0;
        while (i < runtime) {
                sleep(1);
                i++;
                __lich_net_print(args, threads, &begin);
        }

        return 0;
err_ret:
        return ret;
}

typedef struct {
        uint64_t io;
} prof_schedule_ctx_t;

static void __prof_schedule_exec(void *arg)
{
        prof_schedule_ctx_t *ctx = arg;

        ctx->io ++;

        //schedule_task_new("prof_task", __prof_schedule_exec, arg);
}

static void __prof_schedule_dump(prof_schedule_ctx_t *ctx, int threads, uint64_t used)
{
        int i;
        uint64_t io;

        io = 0;
        for (i = 0; i < threads; i++) {
                io += ctx[i].io;
                ctx[i].io = 0;
        }
                        
        printf("iops : %llu\n", (LLU)io * 1000 * 1000 / used);
}

int prof_schedule(int threads)
{
        int ret, efd, i;
        schedule_t *schedule;
        prof_schedule_ctx_t *array;
        time_t last, now;
        struct timeval _last, _now;
        uint64_t used;

        ret = schedule_init();
        if (ret)
                GOTO(err_ret, ret);
        
        ret = schedule_create(&efd, "prof_schedule", 100, &schedule);
        if (ret)
                GOTO(err_ret, ret);

        ret = ymalloc((void **)&array, sizeof(*array) * threads);
        if (ret)
                GOTO(err_ret, ret);

        //printf("init thread\n");
        
        for (i = 0; i < threads; i++) {
                array[i].io = 0;

                schedule_task_new("prof_task", __prof_schedule_exec, &array[i], -1);
        }

        last = gettime();
        _gettimeofday(&_last, NULL);
#if 0
#else

        while (1) {
                schedule_run();

                for (i = 0; i < threads; i++) {
                        schedule_task_new("prof_task", __prof_schedule_exec, &array[i], -1);
                }
                
                now = gettime();
                //printf("now %u, last %u\n", (int)now, (int)last);
                if (now - last >= 1) {
                        //printf("dump iops\n");
                        _gettimeofday(&_now, NULL);

                        used = _time_used(&_last, &_now);

                        __prof_schedule_dump(array, threads, used);

                        last = gettime();
                        _gettimeofday(&_last, NULL);
                }
        }
#endif

        return 0;
err_ret:
        return ret;
}

int main(int argc, char *argv[])
{
        int ret, op = OP_NULL, direct = 0;
        const char *nodename = NULL, *sendsize = "0", *recvsize = "0",
                *thread = "1", *runtime = "30", *rw = NULL,
                *size = "4096", *volume = "/lichbd_prof/pool/device0";
        char c_opt;

        dbg_info(1);

        while (srv_running) {
                int option_index = 0;

                static struct option long_options[] = {
                        { "sendsize", required_argument, 0, 0},
                        { "recvsize", required_argument, 0, 0},
                        { "thread", required_argument, 0, 0},
                        { "runtime", required_argument, 0, 0},
                        { "direct", no_argument, 0, 0},
                        { "volume", required_argument, 0, 0},
                        { "schedule", no_argument, 0, 's'},
                        { "rpc", required_argument, 0, 'r'},
                        { "net", required_argument, 0, 'n'},
                        { "vm", required_argument, 0, 'm'},
                        { "dio", required_argument, 0, 'd'},
                        { "lichbd", required_argument, 0, 'l'},
                        { "verbose", 0, 0, 'v' },
                        { "help",    0, 0, 'h' },
                        { 0, 0, 0, 0 },
                };

                c_opt = getopt_long(argc, argv, "vhn:", long_options, &option_index);
                if (c_opt == -1)
                        break;

                switch (c_opt) {
                case 0:
                        switch (option_index) {
                        case 0:
                                sendsize = optarg;
                                break;
                        case 1:
                                recvsize = optarg;
                                break;
                        case 2:
                                thread = optarg;
                                break;
                        case 3:
                                runtime = optarg;
                                break;
                        case 4:
                                direct = 1;
                                break;
                        case 5:
                                volume = optarg;
                                break;
                        default:
                                fprintf(stderr, "Hoops, wrong op got!\n");
                                YASSERT(0); 
                        }

                        break;
                case 'r':
                        op = OP_RPC;
                        nodename = optarg;
                        break;
                case 'n':
                        op = OP_NET;
                        nodename = optarg;
                        break;
                case 'm':
                        op = OP_VM;
                        nodename = optarg;
                        break;
                case 'd':
                        op = OP_DIO;
                        nodename = optarg;
                        break;
                case 's':
                        op = OP_SCHEDULE;
                        break;
                case 'l':
                        op = OP_LICHBD;
                        rw = optarg;
                        YASSERT(strlen(rw) == 1);
                        YASSERT(rw[0] == 'r' || rw[0] == 'w');
                        break;
                }
        }

        dbg_info(0);

#if 0
        ret = env_init_simple("lich.prof");
        if (ret)
                GOTO(err_ret, ret);
#endif

        switch (op) {
        case OP_RPC:
                if (direct) {
                        ret = __lich_rpc_direct(nodename, sendsize, recvsize, thread, runtime);
                        if (ret)
                                GOTO(err_ret, ret);
                } else {
                        ret = __lich_rpc(nodename, sendsize, recvsize, thread, runtime);
                        if (ret)
                                GOTO(err_ret, ret);
                }
                break;
        case OP_NET:
                ret = prof_net(nodename, atoi(thread), atoi(runtime));
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_VM:
                ret = prof_vm(nodename, atoi(thread), atoi(runtime));
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_DIO:
                ret = prof_dio(nodename, atoi(thread), atoi(runtime));
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_LICHBD:
                ret = prof_lichbd(atoi(thread), atoi(size), atoi(runtime), rw[0], volume);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_SCHEDULE:
                ret = prof_schedule(atoi(thread));
                if (ret)
                        GOTO(err_ret, ret);

                break;
        default:
                usage();
                EXIT(EINVAL);
        }

        //DBUG("test...........\n");

        return 0;
err_ret:
        exit(ret);
}
