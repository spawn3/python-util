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
#include "license.h"
#include "lichbd.h"
#include "net_global.h"
#include "utils.h"
#include "dbg.h"

typedef enum {
        OP_NULL,
        OP_TOUCH,
        OP_COPY,
        OP_MKDIR,
        OP_LIST,
        OP_FIND,
        OP_UNLINK,
        OP_RMDIR,
        OP_STAT,
        OP_RENAME,
        OP_CAT,
        OP_WRITE,
        OP_ATTR_GET,
        OP_ATTR_SET,
        OP_ATTR_REMOVE,
        OP_TRUNCATE,
        OP_FSSTAT,
} admin_op_t;

int object_md5sum(const fileid_t *oid);

static void usage()
{
        fprintf(stderr, "\nusage:\n"
                "lichfs --stat <path>\n"
                "lichfs --mkdir <path>\n"
                "lichfs --touch <path>\n"
                "lichfs --list <path>\n"
                "lichfs --unlink <path>\n"
                "lichfs --rmdir <path>\n"
                "lichfs --find <path> [name]\n"
                "lichfs --copy :<src> <dist> [--single]\n"
                "lichfs --copy <src> :<dist> [--single]\n"
                "lichfs --copy <src> <dist> [--single]\n"
                "lichfs --rename <from> <to>\n"
                "lichfs --cat <path>\n"
                "lichfs --attrset <path> <key> <value>\n"
                "lichfs --attrget <path> <key>\n"
                "lichfs --attrremove <path> <key>\n"
                "lichfs --truncate <path> <length>\n"
                "lichfs --fsstat\n"
               );
}

static int __lich_xattr_set(const char *path, const char *key, const char *value)
{
        return lich_xattr_set(path, key, value);
}

static int __lich_xattr_get(const char *path, const char *key)
{
        return lich_xattr_get(path, key);
}

static int __lich_xattr_remove(const char *path, const char *key)
{
        return lich_xattr_remove(path, key);
}

int main(int argc, char *argv[])
{
        int ret, op = OP_NULL, multithreading = 1;
        char c_opt;
        int verbose = 0;
        size_t size = 0;
        const char *key = NULL, *from = NULL, *to = NULL, *path = NULL, *value = NULL;

        dbg_info(0);

        (void) verbose;

        while (srv_running) {
                int option_index = 0;

                static struct option long_options[] = {
                        { "rename", required_argument, 0, 0},
                        { "cat", required_argument, 0, 0},
                        { "write", required_argument, 0, 0},
                        { "attrset", required_argument, 0, 0},
                        { "attrget", required_argument, 0, 0},
                        { "attrremove", required_argument, 0, 0},
                        { "fsstat", no_argument, 0, 0},
                        { "truncate", required_argument, 0, 0},
                        { "single", no_argument, 0, 0},
                        { "stat", required_argument, 0, 's'},
                        { "mkdir", required_argument, 0, 'm'},
                        { "touch", required_argument, 0, 't'},
                        { "list", required_argument, 0, 'l'},
                        { "find", required_argument, 0, 'f'},
                        { "unlink", required_argument, 0, 'u'},
                        { "rmdir", required_argument, 0, 'r'},
                        { "copy", required_argument, 0, 'c'},
                        { "verbose", 0, 0, 'v' },
                        { "help",    0, 0, 'h' },
                        { 0, 0, 0, 0 },
                };

                c_opt = getopt_long(argc, argv, "s:c:m:t:l:f:u:r:nvh", long_options, &option_index);
                if (c_opt == -1)
                        break;

                switch (c_opt) {
                case 0:
                        switch (option_index) {
                        case 0:
                                op = OP_RENAME;
                                from = optarg;
                                to = argv[3];
                                break;
                        case 1:
                                op = OP_CAT;
                                key = optarg;
                                break;
                        case 2:
                                op = OP_WRITE;
                                from = argv[2];
                                to = argv[3];
                                break;
                        case 3:
                                op = OP_ATTR_SET;
                                path = argv[2];
                                key = argv[3];
                                value = argv[4];
                                break;
                        case 4:
                                op = OP_ATTR_GET;
                                path = argv[2];
                                key = argv[3];
                                break;
                        case 5:
                                op = OP_ATTR_REMOVE;
                                path = argv[2];
                                key = argv[3];
                                break;
                        case 6:
                                op = OP_FSSTAT;
                                //path = argv[2];
                                break;
                        case 7:
                                op = OP_TRUNCATE;
                                path = argv[2];
                                value = argv[3];
                                break;
                        case 8:
                                multithreading = 0;
                                break;
                        default:
                                fprintf(stderr, "Hoops, wrong op got!\n");
                                YASSERT(0); 
                        }

                        break;
                case 's':
                        op = OP_STAT;
                        key = optarg;
                        break;
                case 'm':
                        op = OP_MKDIR;
                        key = optarg;
                        break;
                case 't':
                        op = OP_TOUCH;
                        key = optarg;
                        break;
                case 'l':
                        op = OP_LIST;
                        key = optarg;
                        break;
                case 'f':
                        op = OP_FIND;
                        path = optarg;
                        key = argv[3];
                        break;
                case 'r':
                        op = OP_RMDIR;
                        key = optarg;
                        break;
                case 'u':
                        op = OP_UNLINK;
                        key = optarg;
                        break;
                case 'c':
                        op = OP_COPY;
                        from = optarg;
                        to = argv[3];
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

#if 0
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
#endif

        ret = lichbd_init("");
        if (ret)
                GOTO(err_ret, ret);
        
        if ((op == OP_COPY) || (op == OP_ATTR_SET) || (op == OP_ATTR_REMOVE)) {
                ret = storage_license_check();
                if (ret)
                        GOTO(err_ret, ret);
        }

        switch (op) {
        case OP_MKDIR:
                ret = utils_mkpool(key);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_TOUCH:
                ret = utils_touch(key);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_RMDIR:
                ret = utils_rmpool(key);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_UNLINK:
                ret = utils_rmvol(key);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_LIST:
                ret = utils_list(key, 2);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_FIND:
                ret = utils_find(path, key);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_STAT:
                ret = utils_stat(key, 0);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_COPY:
                ret = stor_copy(from, to, multithreading);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_RENAME:
                ret = utils_rename(from, to);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case OP_CAT:
                ret = utils_cat(key);
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_WRITE:
                ret = utils_write(from, to);
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_ATTR_GET:
                ret = __lich_xattr_get(path, key);
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_ATTR_SET:
                ret = __lich_xattr_set(path, key, value);
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_ATTR_REMOVE:
                ret = __lich_xattr_remove(path, key);
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_FSSTAT:
                ret = utils_fsstat("/");
                if (ret)
                        GOTO(err_ret, ret);
                break;
        case OP_TRUNCATE:
                ret = utils_get_size(value, &size);
                if (ret)
                        GOTO(err_ret, ret);

                ret = utils_truncate(path, size);
                if (ret)
                        GOTO(err_ret, ret);

                printf("truncate %s to %s\n", path, value);
                break;
        default:
                usage();
                exit(EINVAL);
        }

        return 0;
err_ret:
        EXIT(_errno(ret));
}
