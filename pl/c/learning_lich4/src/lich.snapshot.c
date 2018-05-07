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
#include "lich_md.h"
#include "storage.h"
#include "volume.h"
#include "license.h"
#include "md_map.h"
#include "net_global.h"
#include "utils.h"
#include "dbg.h"

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

typedef enum {
        OP_NULL,
        OP_CREATE,
        OP_LIST,
        OP_CAT,
        OP_COPY,
        OP_ROLLBACK,
        OP_REMOVE,
        OP_CLONE,
        OP_PROTECT,
        OP_UNPROTECT,
} snap_op_t;

static void usage()
{
        fprintf(stderr, "\nusage:\n"
                "lich.snapshot --create [file name]@[snap name]\n"
                "lich.snapshot --rollback [file name]@[snap name]\n"
                "lich.snapshot --remove [file name]@[snap name]\n"
                "lich.snapshot --protect [file name]@[file name]\n"
                "lich.snapshot --unprotect [file name]@[file name]\n"
                "lich.snapshot --list [file name]\n"
                "lich.snapshot --clone [file name]@[snap name] [filename]\n"
                //"lich.snapshot --cat [file name]@[snap name]\n"
                "lich.snapshot --copy [file name]@[snap name] :<path>\n"
               );
}

int main(int argc, char *argv[])
{
        int ret, op = OP_NULL;
        char c_opt;
        int verbose = 0, all = 0;
        const char *name = NULL, *to = NULL;

        dbg_info(0);

        (void) verbose;

        while (srv_running) {
                int option_index = 0;

                static struct option long_options[] = {
                        { "cat", required_argument, 0, 0},
                        { "protect", required_argument, 0, 0},
                        { "unprotect", required_argument, 0, 0},
                        { "copy", required_argument, 0, 0},
                        { "create", required_argument, 0, 'c'},
                        { "list", required_argument, 0, 'l'},
                        { "destroy", required_argument, 0, 'd'},
                        { "rollback", required_argument, 0, 'r'},
                        { "remove", required_argument, 0, 'm'},
                        { "clone", required_argument, 0, 'o'},
                        { "send", required_argument, 0, 's'},
                        { "recv", required_argument, 0, 'e'},
                        { "all", 0, 0, 'a'},
                        { "normal", 0, 0, 'n'},
                        { "verbose", 0, 0, 'v' },
                        { "help",    0, 0, 'h' },
                        { 0, 0, 0, 0 },
                };

                c_opt = getopt_long(argc, argv, "c:l:d:r:s:e:anvh", long_options, &option_index);
                if (c_opt == -1)
                        break;

                switch (c_opt) {
                case 0:
                        switch (option_index) {
                        case 0:
                                op = OP_CAT;
                                name = optarg;
                                break;
                        case 1:
                                op = OP_PROTECT;
                                name = optarg;
                                break;
                        case 2:
                                op = OP_UNPROTECT;
                                name = optarg;
                                break;
                        case 3:
                                op = OP_COPY;
                                name = optarg;
                                to = argv[3];
                                break;
                        default:
                                fprintf(stderr, "Hoops, wrong op got!\n");
                                YASSERT(0); 
                        }

                        break;
                case 'c':
                        op = OP_CREATE;
                        name = optarg;
                        break;
                case 'l':
                        op = OP_LIST;
                        name = optarg;
                        break;
                case 'r':
                        op = OP_ROLLBACK;
                        name = optarg;
                        break;
                case 'o':
                        op = OP_CLONE;
                        name = optarg;
                        to = argv[3];
                        break;
                case 'm':
                        op = OP_REMOVE;
                        name = optarg;
                        break;
                case 'a':
                        all = 1;
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

#if 1
        if (argc == 1) {
                usage();
                exit(EINVAL);
        }
#endif

        ret = env_init_simple("lich");
        if (ret)
                GOTO(err_ret, ret);

        ret = network_connect_master();
        if (ret) {
            GOTO(err_ret, ret);
        }

        ret = stor_init(NULL, -1);
        if (ret)
                GOTO(err_ret, ret);

        ret = storage_license_check();
        if (ret)
                GOTO(err_ret, ret);

        switch (op) {
        case OP_CREATE:
                ret = utils_snapshot(name);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_LIST:
                ret = utils_snapshot_list(name, all, 0);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_CAT:
                ret = utils_snapshot_cat(name);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_COPY:
                ret = utils_snapshot_copy(name, to);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_ROLLBACK:
                ret = utils_snapshot_rollback(name);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_REMOVE:
                ret = utils_snapshot_remove(name);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_CLONE:
                ret = utils_snapshot_clone(name, to);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_PROTECT:
                ret = utils_snapshot_protect(name);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_UNPROTECT:
                ret = utils_snapshot_unprotect(name);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        default:
                usage();
                exit(EINVAL);
        }

        return 0;
err_ret:
        EXIT(_errno(ret));
}
