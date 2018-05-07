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
#include "cluster.h"
#include "lich_md.h"
#include "lichstor.h"
#include "license.h"
#include "md_map.h"
#include "lichstor.h"
#include "net_global.h"
#include "utils.h"
#include "dbg.h"

typedef enum {
        OP_NULL,
        OP_SET,
        OP_LIST,
        OP_GET,
        OP_REMOVE,
} admin_op_t;

int object_md5sum(const fileid_t *oid);

static void usage()
{
        fprintf(stderr, "\nusage:\n"
                "lich.attr --set key --value value  <path>\n"
                "lich.attr --get key <path>\n"
                "lich.attr --remove key <path>\n"
                "lich.attr --list <path>\n"
               );
}

int lich_xattr_set(const char *path, const char *key, const char *value)
{
        int ret;
        fileid_t fileid;
        nid_t nid;

        if (key == NULL || path == NULL || value == NULL) {
                usage();
                EXIT(EINVAL);
        }

        if (strcmp(key, LICH_SYSTEM_ATTR_PASSWORD) == 0) {
                if (strlen(value) < 12 || strlen(value) > 16) {
                        ret = EINVAL;
                        GOTO(err_ret, ret);
                }
        } else if (strcmp(key, LICH_SYSTEM_ATTR_CREATETIME) == 0) {
                ret = EPERM;
                GOTO(err_ret, ret);
        }

        DBUG("set %s %s %s\n", key, value, path);

        ret = stor_lookup1(path, &fileid);
        if (ret) {
                GOTO(err_ret, ret);
        }

        if (strcmp(key, LICH_SYSTEM_ATTR_REPNUM) == 0) {
                ret = stor_set_repnum(&fileid, atoi(value));
                if (ret)
                        GOTO(err_ret, ret);
        } else {
                ret = md_map_getsrv(&fileid, &nid);
                if (ret)
                        GOTO(err_ret, ret);

                ret = md_xattr_set(&nid, &fileid, key, value, strlen(value), 0);
                if (ret)
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

int lich_xattr_get(const char *path, const char *key)
{
        int ret, buflen;
        char buf[MAX_BUF_LEN];
        fileid_t fileid;
        fileinfo_t fileinfo;
        nid_t nid;

        if (key == NULL || path == NULL) {
                usage();
                EXIT(EINVAL);
        }

        ret = stor_lookup1(path, &fileid);
        if (ret) {
                GOTO(err_ret, ret);
        }

        ret = md_getattr(&fileid, &fileinfo);
        if (ret)
                GOTO(err_ret, ret);

        if (strcmp(key, LICH_SYSTEM_ATTR_REPNUM) == 0) {
                snprintf(buf, MAX_BUF_LEN, "%d", fileinfo.repnum);
        } else {
                buflen = MAX_BUF_LEN;

                ret = md_map_getsrv(&fileid, &nid);
                if (ret)
                        GOTO(err_ret, ret);

                ret = md_xattr_get(&nid, &fileid, key, buf, &buflen);
                if (ret) {
                        GOTO(err_ret, ret);
                }
        }

        printf("%s\n", buf);

        return 0;
err_ret:
        return ret;
}

int lich_xattr_remove(const char *path, const char *key)
{
        int ret;
        fileid_t fileid;
        nid_t nid;

        if (key == NULL || path == NULL) {
                usage();
                EXIT(EINVAL);
        }

        ret = stor_lookup1(path, &fileid);
        if (ret) {
                GOTO(err_ret, ret);
        }

        ret = md_map_getsrv(&fileid, &nid);
        if (ret)
                GOTO(err_ret, ret);

        ret = md_xattr_remove(&nid, &fileid, key);
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

#if 0
#define attr_for_each(buf, buflen, de, off)                      \
        YASSERT(sizeof(off) == sizeof(uint64_t));               \
        for (de = (void *)(buf);                                \
             (void *)de < (void *)(buf) + buflen ;              \
             off = de->d_off, de = (void *)de + de->d_reclen)

static int __lich_attr_list(const char *path)
{
        int ret;
        attrlist_t *list;

        ret = ENOSYS;
        GOTO(err_ret, ret);

        if (path == NULL) {
                usage();
                EXIT(EINVAL);
        }

        while (1) {
                ret = storage_attr_list(path, key, strlen(value));
                if (ret)
                        GOTO(err_ret, ret);

                attr_for_each () {
                }
        }

        return 0;
err_ret:
        return ret;
}

#endif


#if 0
int main(int argc, char *argv[])
{
        int ret, op = OP_NULL;
        char c_opt;
        int verbose = 0;
        const char *key = NULL, *value = NULL;

        dbg_info(0);

        (void) verbose;

        while (srv_running) {
                int option_index = 0;

                static struct option long_options[] = {
                        { "set", required_argument, 0, 's'},
                        { "get", required_argument, 0, 'g'},
                        { "value", required_argument, 0, 'V'},
                        { "list", required_argument, 0, 'l'},
                        { "remove", required_argument, 0, 'r'},
                        { "verbose", 0, 0, 'v' },
                        { "help",    0, 0, 'h' },
                        { 0, 0, 0, 0 },
                };

                c_opt = getopt_long(argc, argv, "c:l:eas:mr:vh", long_options, &option_index);
                if (c_opt == -1)
                        break;

                switch (c_opt) {
                case 0:
                        switch (option_index) {
                        default:
                                fprintf(stderr, "Hoops, wrong op got!\n");
                                YASSERT(0); 
                        }

                        break;
                case 's':
                        op = OP_SET;
                        key = optarg;
                        break;
                case 'g':
                        op = OP_GET;
                        key = optarg;
                        break;
                case 'V':
                        value = optarg;
                        break;
                case 'l':
                        DBUG("list\n");
                        op = OP_LIST;
                        key = optarg;
                        break;
                case 'r':
                        op = OP_REMOVE;
                        key = optarg;
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
                EXIT(EINVAL);
        }
#endif

        ret = env_init_simple("lich.attr");
        if (ret)
                GOTO(err_ret, ret);

        ret = network_connect_master();
        if (ret) {
            GOTO(err_ret, ret);
        }

        ret = stor_init(NULL, -1);
        if (ret)
                GOTO(err_ret, ret);

        if((op == OP_SET) || (op == OP_REMOVE)) {
                ret = storage_license_check();
                if (ret)
                        GOTO(err_ret, ret);
        }

        //DBUG("type %d op %d\n", type, op);

        //fence_test();

        switch (op) {
        case OP_SET:
                ret = __lich_xattr_set(argv[argc - 1], key, value);
                if (ret) {
                        GOTO(err_ret, ret);
                }

                break;
        case OP_GET:
                ret = __lich_xattr_get(argv[argc - 1], key);
                if (ret) {
                        GOTO(err_ret, ret);
                }

                break;
        case OP_REMOVE:
                ret = __lich_xattr_remove(argv[argc - 1], key);
                if (ret) {
                        GOTO(err_ret, ret);
                }

                break;
        default:
                usage();
                EXIT(EINVAL);
        }

        return 0;
err_ret:
        EXIT(_errno(ret));
}

#endif
