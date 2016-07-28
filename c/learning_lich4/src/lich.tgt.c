#include "config.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <dirent.h>
#include <ctype.h>

#include "configure.h"
#include "license.h"
#include "yiscsi.h"
#include "block.h"
#include "lichstor.h"
#include "storage.h"
#include "adt.h"

#define DEFAULT    "default"

#define SIZE_1G (1000 * 1000 * 1000)
#define SIZE_1M (1000 * 1000)
#define SIZE_1K (1000)

/*
 * Global variable
 */
int verbose = 0;

#define ISCSI_USER_MAX          20
#define ISCSI_LUN_ALIAS_MAX     20

static void usage()
{
        fprintf(stderr,
                "usage: lich.tgt [-verbose | -v] [--help | -h]\n"
                "               [--mode | -m mode]\n"
                "               [--op | -o op]\n"
                "               [--namespace | -t namespace]\n"
                "               [--target | -t target]\n"
                "               [--lun | -l lun]\n"
                "               [--alias | -a alias]\n"
                "               [--size | -s size]\n"
                "               [--user|-u user]\n"
                "               [--pass|-p pass]\n"
                "               [--direction|-d dir]\n"
                "\n"
                "-v --verbose           Show verbose message\n"
                "-h --help              Show this help\n"
                "-o --op=op             Valid op: `new' `del' `list'\n"
                "-m --mode=mode         Valid mode: `target' `lun' `account'\n"
                "-n --namespace=namespace The namespace string\n"
                "-t --target=target     The target name string\n"
                "-l --lun=lun           The of logic unit, valid is 0~%u\n"
                "-a --alias=alias       The alias of lun, length is 0~%u\n"
                "-s --size=size         Valid unit: [b|B] [k|K] [m|M] [g|G]\n"
                "-u --user=user         The username to set\n"
                "-p --pass=pass         The password to set\n"
                "-d --direction=dir     The direction of the account to set\n"
                "\n"
                "See `manual of the yiscsi' for more detail.\n"
                "\n",
                ISCSI_LUN_MAX, ISCSI_LUN_ALIAS_MAX
               );
}

/*
 * Options structure for parse
 */
static struct option long_options[] = {
        { "verbose",    0, 0, 'v' },
        { "help",       0, 0, 'h' },
        { "op",         1, 0, 'o' },
        { "mode",       1, 0, 'm' },
        { "namespace",  1, 0, 'n' },
        { "target",     1, 0, 't' },
        { "lun",        1, 0, 'l' },
        { "alias",      1, 0, 'a' },
        { "size",       1, 0, 's' },
        { "user",       1, 0, 'u' },
        { "pass",       1, 0, 'p' },
        { "direction",  1, 0, 'd' },
        { 0, 0, 0, 0 },
};

const char optstr[] = "vho:m:n:t:l:a:s:u:p:d:";

/*
 * Options enum and tokens for convert
 */
struct ytoken {
        char *key;
        int val;
};

enum tgtop_t {
        OP_NEW,
        OP_DEL,
        OP_LIST,
};

enum tgtmode_t {
        MODE_TARGET,
        MODE_LUN,
        MODE_ALIAS,
        MODE_ACCOUNT,
};

enum tgtdir_t {
        DIR_IN,
        DIR_OUT,
};

struct ytoken target_op_tokens[] = {
        { "new",  OP_NEW  },
        { "del",  OP_DEL  },
        { "list", OP_LIST },
};

struct ytoken lun_op_tokens[] = {
        { "new",  OP_NEW  },
        { "del",  OP_DEL  },
        { "list", OP_LIST },
};

struct ytoken account_op_tokens[] = {
        { "new",  OP_NEW  },
        { "del",  OP_DEL  },
        { "list", OP_LIST },
};

struct ytoken alias_op_tokens[] = {
        { "new",  OP_NEW  },
        { "del",  OP_DEL  },
        { "list", OP_LIST },
};

struct ytoken mode_tokens[] = {
        { "target",  MODE_TARGET },
        { "lun",     MODE_LUN },
        { "account", MODE_ACCOUNT },
        { "alias",   MODE_ALIAS },
};

struct ytoken tgtdir_tokens[] = {
        { "in",  DIR_IN },
        { "out", DIR_OUT },
};

struct ychap_key {
        char *key_user;
        char *key_pass;
};

struct ychap_key ychap_keys[] = {
        { "iscsi.in_user", "iscsi.in_pass" },
        { "iscsi.out_user", "iscsi.out_pass" },
};

#define IQN_FORMAT      "+----------------------------------------------+\n"    \
                        "|                   IQN                        |\n"    \
                        "|----------------------------------------------|\n"    \
                        "| %44s |\n"                                            \
                        "+----------------------------------------------+\n"

#define TGT_SPLIT      "\n+----------------------------------------------+\n"   \
                        "| %-44s |\n"                                           \
                        "+----------------------------------------------+\n"

#define TGT_FORMAT      "\nTarget (%s) :                                 \n"
#define NS_FORMAT       "\nNamespace (%s) :                              \n"

#define LUN_TBL_BEGIN   "+----------------------------------------------+\n"    \
                        "|    Lun    |    Size   |         Alias        |\n"    \
                        "|-----------+-----------+----------------------|\n"
#define LUN_TBL_FORMAT  "| %9s | %8.3f%c | %20s |\n"
#define LUN_TBL_END     "+----------------------------------------------+\n"

#define ALIAS_FORMAT    "+----------------------------------------------+\n"    \
                        "|    Lun    |   Alias                          |\n"    \
                        "|-----------+----------------------------------|\n"    \
                        "| %9s | %32s |\n"                                      \
                        "+----------------------------------------------+\n"

#define ACCOUNT_FORMAT  "+----------------------------------------------+\n"    \
                        "|      User (In)       |     Pass (In)         |\n"    \
                        "|----------------------+-----------------------|\n"    \
                        "| %20s | %21s |\n"                                     \
                        "|----------------------------------------------|\n"    \
                        "|      User (Out)      |     Pass (Out)        |\n"    \
                        "|----------------------+-----------------------|\n"    \
                        "| %20s | %21s |\n"                                     \
                        "+----------------------------------------------+\n"

#define ytoken_size(tokens) (sizeof(tokens)/sizeof(tokens[0]))

static inline int ytoken_match(struct ytoken *tokens, int token_cnt, char *key)
{
        int i;

        for (i = 0; i < token_cnt; ++i)
                if (!strcmp(tokens[i].key, key))
                        return tokens[i].val;

        return -1;
}

/**
 * Misc functions
 */

static int check_lun_name(char *name)
{
        int ret;
        size_t i;
        uint32_t lun;

        for (i = 0; i < strlen(name); ++i) {
                if (!isdigit(name[i])) {
                        ret = EINVAL;
                        GOTO(err_ret, ret);
                }
        }

        lun = atoll(name);

        if (lun > ISCSI_LUN_MAX) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int check_target_name(char *ns, char *tname)
{
        int ret;

        ret = conf_init();
        if (ret)
                GOTO(err_ret, ret);

        dbg_sub_init();

        if (ns == NULL || tname == NULL) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        if (!is_valid_name(ns, "namespace")) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        if (!is_valid_name(tname, "target")) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        /*         iqn            : name(namespace . target) */
        if (strlen(sanconf.iqn) + 1 + strlen(ns) + 1 + strlen(tname) > ISCSI_TGT_NAME_MAX) {
                fprintf(stderr, "\x1b[1;31minvalid name length, namespace and target maximum length is %ld\x1b[0m\n",
                        ISCSI_TGT_NAME_MAX - strlen(sanconf.iqn) - 1 - 1);
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        //fprintf(stderr, "the format of target name is: namespace.tname\n");
        return ret;
}

static int get_size(char *str, uint64_t *_size)
{
        int ret;
        uint64_t size = 0;
        char unit;

        if (strlen(str) < 2) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

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

static int uss_lun_list(fileid_t *tgtid, int multilayer)
{
        int ret, delen;
        off_t offset;
        char *de0;
        struct dirent *de;
        char lname[MAX_PATH_LEN], unit;
        char alias[MAX_BUF_LEN];
        double size;
        fileid_t oid;
        struct stat stbuf;

        printf(LUN_TBL_BEGIN);

        delen = BIG_BUF_LEN;
        ret = ymalloc((void **)&de0, delen);
        if (ret)
                GOTO(err_ret, ret);

        offset = 0;
        while (1) {
                ret = block_listpool(tgtid, offset, de0, &delen);
                if (ret)
                        GOTO(err_free, ret);
                else if (delen == 0)
                        break;

                dir_for_each(de0, delen, de, offset) {
                        if (strlen(de->d_name) == 0)
                                goto out;

                        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..") ||
                            /* Skip Lun if the name start with '.' */
                            de->d_name[0] == '.') {
                                continue;
                        }

                        if (multilayer) {
                                if (check_lun_name(de->d_name))
                                        continue;
                        }

                        ret = block_lookup(tgtid, de->d_name, &oid);
                        if (ret)
                                continue;

                        if (!multilayer) {
                                if (oid.type == __POOL_CHUNK__)
                                        continue;
                        }

                        ret = block_getattr(&oid, &stbuf);
                        if (ret)
                                continue;

                        /* Alias is optional */
                        alias[0] = 0;
                        block_xattr_get(NULL, &oid, yiscsi_lun_alias_key, alias, NULL);
                        if (!strcmp(alias, yiscsi_none_value))
                                alias[0] = 0;

                        snprintf(lname, MAX_PATH_LEN, "%s", de->d_name);

                        if (stbuf.st_size > SIZE_1G) {
                                size = stbuf.st_size * 1.0 / SIZE_1G;
                                unit = 'G';
                        } else if (stbuf.st_size > SIZE_1M) {
                                size = stbuf.st_size * 1.0 / SIZE_1M;
                                unit = 'M';
                        } else if (stbuf.st_size > SIZE_1K) {
                                size = stbuf.st_size * 1.0 / SIZE_1K;
                                unit = 'K';
                        } else {
                                size = stbuf.st_size;
                                unit = 'B';
                        }

                        printf(LUN_TBL_FORMAT, lname, size, unit, alias);
                }
        }
out:
        yfree((void **)&de0);
        printf(LUN_TBL_END);
        return 0;
err_free:
        yfree((void **)&de0);
err_ret:
        return ret;
}

static int uss_tgt_list(const fileid_t *nsid, const char *ns, const char *filter)
{
        int ret, delen;
        off_t offset;
        char *de0;
        struct dirent *de;
        char tname[MAX_PATH_LEN];
        char value[MAX_BUF_LEN];
        fileid_t oid;

        delen = BIG_BUF_LEN;
        ret = ymalloc((void **)&de0, delen);
        if (ret)
                GOTO(err_ret, ret);

        offset = 0;
        while (1) {
                ret = block_listpool(nsid, offset, de0, &delen);
                if (ret)
                        GOTO(err_free, ret);
                else if (delen == 0)
                        break;

                dir_for_each(de0, delen, de, offset) {
                        if (strlen(de->d_name) == 0)
                                goto out;

                        if (filter && strcmp(filter, de->d_name))
                                continue;

                        ret = block_lookup(nsid, de->d_name, &oid);
                        if (ret)
                                continue;

                        if (oid.type == __VOLUME_CHUNK__)
                                continue;

                        memset(value, 0x00, sizeof(value));

                        snprintf(tname, sizeof(tname), "%s.%s", ns, de->d_name);

                        printf(TGT_FORMAT, tname);

                        (void) uss_lun_list(&oid, 1);
                }
        }

out:
        yfree((void **)&de0);
        return 0;
err_free:
        yfree((void **)&de0);
err_ret:
        return ret;
}

static int __block_root(fileid_t *rootid)
{
        int ret, retry = 0;

retry:
        ret = block_root(rootid);
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

static int uss_ns_list(const char *nf, const char *tf)
{
        int ret, delen;
        uint64_t offset;
        char *de0, path[MAX_BUF_LEN];
        struct dirent *de;
        fileid_t oid;
        fileid_t rootid;

        ret = __block_root(&rootid);
        if (ret)
                GOTO(err_ret, ret);

        delen = BIG_BUF_LEN;
        ret = ymalloc((void **)&de0, delen);
        if (ret)
                GOTO(err_ret, ret);

        offset = 0;
        while (1) {
                ret = block_listpool(&rootid, offset, de0, &delen);
                if (ret)
                        GOTO(err_free, ret);

                if (delen == 0)
                        break;

                dir_for_each(de0, delen, de, offset) {
                        if (strlen(de->d_name) == 0)
                                goto out;

                        if (nf && strcmp(nf, de->d_name))
                                continue;

                        snprintf(path, MAX_BUF_LEN, "/%s", de->d_name);
                        ret = block_lookup(&rootid, de->d_name, &oid);
                        if (ret)
                                continue;

                        (void) uss_tgt_list(&oid, de->d_name, tf);

                        if (strlen(de->d_name) == 0)
                                break;
                }
        }

out:
        yfree((void **)&de0);
        return 0;
err_free:
        yfree((void **)&de0);
err_ret:
        return ret;
}

static int uss_ns_list_monolayer(const char *nf)
{
        int ret, delen;
        uint64_t offset;
        char *de0, path[MAX_BUF_LEN];
        struct dirent *de;
        fileid_t oid;
        fileid_t rootid;

        ret = __block_root(&rootid);
        if (ret)
                GOTO(err_ret, ret);

        delen = BIG_BUF_LEN;
        ret = ymalloc((void **)&de0, delen);
        if (ret)
                GOTO(err_ret, ret);

        offset = 0;
        while (1) {
                ret = block_listpool(&rootid, offset, de0, &delen);
                if (ret)
                        GOTO(err_free, ret);

                if (delen == 0)
                        break;

                dir_for_each(de0, delen, de, offset) {
                        if (strlen(de->d_name) == 0)
                                goto out;

                        if (nf && strcmp(nf, de->d_name))
                                continue;

                        if (!strcmp(DEFAULT, de->d_name))
                                continue;

                        snprintf(path, MAX_BUF_LEN, "/%s", de->d_name);
                        ret = block_lookup(&rootid, de->d_name, &oid);
                        if (ret)
                                continue;

                        if (strlen(de->d_name) == 0)
                                break;

                        printf(NS_FORMAT, de->d_name);
                        (void) uss_lun_list(&oid, 0);
                }
        }

out:
        yfree((void **)&de0);
        return 0;
err_free:
        yfree((void **)&de0);
err_ret:
        return ret;
}

static int __block_mkpool(const fileid_t *parent, const char *name, fileid_t *fid, int ignore_exist)
{
        int ret, retry = 0;

retry0:
        ret = block_mkpool(parent, name, NULL, fid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry0, retry);
                } else if (ret == EEXIST && ignore_exist) {
                        retry = 0;
retry1:
                        ret = block_lookup(parent, name, fid);
                        if (ret) {
                                if (ret == EAGAIN) {
                                        SLEEP_RETRY(err_ret, ret, retry1, retry);
                                } else
                                        GOTO(err_ret, ret);
                        }
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

/**
 * Target operation process
 */

static int mode_target_new(char *ns, char *tname, fileid_t *id)
{
        int ret;
        fileid_t oid, rootid, nsid;
        char path[MAX_PATH_LEN];

        snprintf(path, sizeof(path), "/%s/%s", ns, tname);

        ret = __block_root(&rootid);
        if (ret)
                GOTO(err_ret, ret);

        ret = __block_mkpool(&rootid, ns, &nsid, 1);
        if (ret)
                GOTO(err_ret, ret);

        ret = __block_mkpool(&nsid, tname, &oid, 0);
        if (ret)
                GOTO(err_ret, ret);

        if (id)
                *id = oid;

        return 0;
err_ret:
        return ret;
}

static int mode_target_del(char *ns, char *tname)
{
        int ret, retry = 0;
        char path[MAX_PATH_LEN];
        fileid_t nsid, tgtid;

        sprintf(path, "/%s", ns);
retry:
        ret = block_lookup1(path, &nsid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        retry= 0;
retry1:
        ret = block_lookup(&nsid, tname, &tgtid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry1, retry);
                } else
                        GOTO(err_ret, ret);
        }

        if (tgtid.type == __VOLUME_CHUNK__) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        retry= 0;
retry2:
        ret = block_rmdir(&nsid, tname);
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

static int mode_target_list()
{
        int ret;

        printf(IQN_FORMAT, sanconf.iqn);
        printf(TGT_SPLIT, "multilayer target");
        ret = uss_ns_list(DEFAULT, NULL);
        if (ret)
                GOTO(err_ret, ret);

        printf(TGT_SPLIT, "monolayer target");
        ret = uss_ns_list_monolayer(NULL);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int __block_create(const fileid_t *parent, const char *name, fileid_t *fid, uint64_t size)
{
        int ret, retry = 0;
        struct stat stbuf;

retry:
        ret = stor_mkvol(parent, name, NULL, fid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else if (ret == EEXIST) {
                        if (retry > 0) {
                                ret = stor_lookup(parent, name, fid);
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

        retry = 0;
retry1:
        ret = stor_truncate(fid, size);
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

/**
 * Lun operation process
 */

static int mode_lun_new_multilayer(char *ns, char *tname, char *lname, char *alias, uint64_t size)
{
        int ret;
        char tgt[MAX_NAME_LEN];
        fileid_t lunid, tgtid;

        (void) alias;

        ret = mode_target_new(ns, tname, &tgtid);
        if (ret) {
                if (ret == EEXIST) {
                        snprintf(tgt, sizeof(tgt), "/%s/%s", ns, tname);
                        ret = block_lookup1(tgt, &tgtid);
                        if (ret)
                                GOTO(err_ret, ret);
                } else
                        GOTO(err_ret, ret);
        }

        ret = __block_create(&tgtid, lname, &lunid, size);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int mode_lun_new_monolayer(char *ns, char *tname, char *alias, uint64_t size)
{
        int ret;
        fileid_t rootid, nsid, tgtid;

        (void) alias;

        ret = __block_root(&rootid);
        if (ret)
                GOTO(err_ret, ret);

        ret = __block_mkpool(&rootid, ns, &nsid, 1);
        if (ret) {
                GOTO(err_ret, ret);
        }

        ret = __block_create(&nsid, tname, &tgtid, size);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int mode_lun_del_multilayer(char *ns, char *tname, char *lname)
{
        int ret, retry;
        char tgt[MAX_NAME_LEN];
        fileid_t tgtid;

        snprintf(tgt, sizeof(tgt), "/%s/%s", ns, tname);
        retry = 0;
retry:
        ret = block_lookup1(tgt, &tgtid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        retry = 0;
retry1:
        ret = block_unlink(&tgtid, lname);
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

static int mode_lun_del_monolayer(char *ns, char *tname)
{
        int ret, retry;
        char tgt[MAX_NAME_LEN];
        fileid_t nsid, tgtid;

        snprintf(tgt, sizeof(tgt), "/%s", ns);
        retry = 0;
retry:
        ret = block_lookup1(tgt, &nsid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        retry = 0;
retry1:
        ret = block_lookup(&nsid, tname, &tgtid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry1, retry);
                } else
                        GOTO(err_ret, ret);
        }

        if (tgtid.type == __POOL_CHUNK__) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        retry = 0;
retry2:
        ret = block_unlink(&nsid, tname);
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

static int mode_lun_list(char *ns, char *subname)
{
        printf(IQN_FORMAT, sanconf.iqn);
        return uss_ns_list(ns, subname);
}

static int mode_lun_list_monolayer(char *ns)
{
        printf(IQN_FORMAT, sanconf.iqn);
        return uss_ns_list_monolayer(ns);
}

/**
 * Alias operation process
 */

static int mode_alias_new(char *ns, char *tname, char *lname, char *alias)
{
        (void) ns;
        (void) tname;
        (void) lname;
        (void) alias;
        UNIMPLEMENTED(__DUMP__);

        return 0;

#if 0
        int ret;
        char path[MAX_PATH_LEN];
        fileid_t fileid;

        snprintf(path, sizeof(path), "/%s/%s/%s", ns, tname, lname);

        ret = raw_lookup1(path, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        ret = raw_setxattr(&fileid, yiscsi_lun_alias_key, alias);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
#endif
}

static int mode_alias_del(char *ns, char *tname, char *lname)
{
        (void) ns;
        (void) tname;
        (void) lname;
        UNIMPLEMENTED(__DUMP__);

        return 0;
#if 0

        int ret;
        char path[MAX_PATH_LEN];
        fileid_t fileid;

        snprintf(path, sizeof(path), "/%s/%s/%s", ns, tname, lname);

        ret = raw_lookup1(path, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        ret = raw_setxattr(&fileid, yiscsi_lun_alias_key, yiscsi_none_value);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
#endif
}

static int mode_alias_list(char *ns, char *tname, char *lname)
{
        (void) ns;
        (void) tname;
        (void) lname;
        UNIMPLEMENTED(__DUMP__);

        return 0;
#if 0

        int ret;
        char path[MAX_PATH_LEN], alias[MAX_PATH_LEN];
        fileid_t fileid;

        snprintf(path, sizeof(path), "/%s/%s/%s", ns, tname, lname);

        ret = raw_lookup1(path, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        alias[0] = 0;

        (void) raw_getxattr(&fileid, yiscsi_lun_alias_key,  alias);

        if (!strcmp(alias, yiscsi_none_value))
                alias[0] = 0;

        printf(ALIAS_FORMAT, lname, alias);

        return 0;
err_ret:
        return ret;
#endif
}

/**
 * Account operation process
 */
static int mode_account_new(char *ns, char *tname,
                            char *user, char *pass, int dir)
{
        (void) ns;
        (void) tname;
        (void) user;
        (void) pass;
        (void) dir;
        UNIMPLEMENTED(__DUMP__);

        return 0;
#if 0
        int ret;
        char path[MAX_PATH_LEN];
        fileid_t fileid;

        snprintf(path, sizeof(path), "/%s/%s", ns, tname);

        ret = raw_lookup1(path, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        ret = raw_setxattr(&fileid, ychap_keys[dir].key_user, user);
        if (ret)
                GOTO(err_ret, ret);

        ret = raw_setxattr(&fileid, ychap_keys[dir].key_pass, pass);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
#endif
}

static int mode_account_del(char *ns, char *tname, int dir)
{
        (void) ns;
        (void) tname;
        (void) dir;
        UNIMPLEMENTED(__DUMP__);

        return 0;
#if 0

        int ret;
        char path[MAX_PATH_LEN];
        fileid_t fileid;

        snprintf(path, sizeof(path), "/%s/%s", ns, tname);

        ret = raw_lookup1(path, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        ret = raw_setxattr(&fileid, ychap_keys[dir].key_user, yiscsi_none_value);
        if (ret)
                GOTO(err_ret, ret);

        ret = raw_setxattr(&fileid, ychap_keys[dir].key_pass, yiscsi_none_value);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
#endif
}

static int mode_account_list(char *ns, char *tname)
{
        (void) ns;
        (void) tname;
        UNIMPLEMENTED(__DUMP__);

        return 0;
#if 0
        int ret;
        char path[MAX_PATH_LEN], user[2][MAX_BUF_LEN], pass[2][MAX_BUF_LEN];
        fileid_t fileid;

        snprintf(path, sizeof(path), "/%s/%s", ns, tname);

        ret = raw_lookup1(path, &fileid);
        if (ret)
                GOTO(err_ret, ret);

        memset(user, 0x00, sizeof(user));
        memset(pass, 0x00, sizeof(pass));

        (void) raw_getxattr(&fileid, ychap_keys[DIR_IN].key_user,  user[0]);
        (void) raw_getxattr(&fileid, ychap_keys[DIR_IN].key_pass,  pass[0]);
        (void) raw_getxattr(&fileid, ychap_keys[DIR_OUT].key_user, user[1]);
        (void) raw_getxattr(&fileid, ychap_keys[DIR_OUT].key_pass, pass[1]);

        if (!strcmp(user[DIR_IN], yiscsi_none_value))
                user[DIR_IN][0] = 0;
        if (!strcmp(pass[DIR_IN], yiscsi_none_value))
                pass[DIR_IN][0] = 0;

        if (!strcmp(user[DIR_OUT], yiscsi_none_value))
                user[DIR_OUT][0] = 0;
        if (!strcmp(pass[DIR_OUT], yiscsi_none_value))
                pass[DIR_OUT][0] = 0;

        printf(ACCOUNT_FORMAT, user[DIR_IN], pass[DIR_IN], user[DIR_OUT], pass[DIR_OUT]);

        return 0;
err_ret:
        return ret;
#endif
}

int main(int argc, char *argv[])
{
        int ret;
        char c_opt;
        char *ns= NULL, *tname = NULL, *lname = NULL, *alias = NULL;
        char *user = NULL, *pass = NULL;
        char *option = NULL, *def = DEFAULT;
        uint64_t size = 0;
        int op = -1, mode = -1, dir = -1;

        dbg_info(0);

        while (srv_running) {
                int option_index = 0;

               c_opt = getopt_long(argc, argv, optstr, long_options, &option_index);
               if (c_opt == -1)
                        break;

                switch (c_opt) {
                case 'v':
                        verbose = 1;
                        break;
                case 'h':
                        usage();
                        exit(0);
                case 'o':
                        option = optarg;
                        break;
                case 'm':
                        mode = ytoken_match(mode_tokens,
                                            ytoken_size(mode_tokens),
                                            optarg);
                        if (mode == -1) {
                                fprintf(stderr, "invalid argument: `%s'\n", optarg);
                                EXIT(EINVAL);
                        }
                        break;
                case 'n':
                        ns= optarg;
                        break;
                case 't':
                        tname = optarg;
                        break;
                case 'l':
                        lname = optarg;
                        ret = check_lun_name(lname);
                        if (ret) {
                                fprintf(stderr, "invalid lun, only digit is allowed, "
                                        "valid lun: 0~%u\n", ISCSI_LUN_MAX);
                                EXIT(EINVAL);
                        }
                        break;
                case 'a':
                        alias = optarg;
                        if (strlen(alias) > ISCSI_LUN_ALIAS_MAX) {
                                fprintf(stderr, "alias too long, max length %u\n",
                                        ISCSI_LUN_ALIAS_MAX);
                                EXIT(EINVAL);
                        }
                        break;
                case 's':
                        ret = get_size(optarg, &size);
                        if (ret || !size) {
                                fprintf(stderr, "invalid argument: `%s'\n", optarg);
                                EXIT(EINVAL);
                        }
                        break;
                case 'u':
                        user = optarg;
                        if (strlen(user) > ISCSI_USER_MAX) {
                                fprintf(stderr, "user too long, max length %u\n",
                                        ISCSI_USER_MAX);
                                EXIT(EINVAL);
                        }
                        break;
                case 'p':
                        pass = optarg;
                        if (strlen(pass) < 12 || strlen(pass) > 16) {
                                fprintf(stderr, "The length of CHAP passwod must be [12 ~ 16] bytes\n");
                                EXIT(EINVAL);
                        }
                        break;
                case 'd':
                        dir = ytoken_match(tgtdir_tokens,
                                           ytoken_size(tgtdir_tokens),
                                           optarg);
                        if (dir == -1) {
                                fprintf(stderr, "invalid argument: `%s'\n", optarg);
                                EXIT(EINVAL);
                        }
                        break;
                default:
                        usage();
                        EXIT(EINVAL);
                }
        }

        /* Arguments check */
        if (mode == -1) {
                fprintf(stderr, "--mode must be specified\n");
                usage();
                exit(EINVAL);
        }

        if (option == NULL) {
                fprintf(stderr, "--op must be specified\n");
                EXIT(EINVAL);
        }

        if (ns == NULL) {
                ns = def;
        }

        ret = env_init_simple("lich");
        if (ret)
                GOTO(err_ret, ret);

        ret = network_connect_master();
        if (ret)
                GOTO(err_ret, ret);

        ret = stor_init(NULL, -1);
        if (ret)
                GOTO(err_ret, ret);

        ret = block_init();
        if (ret)
                GOTO(err_ret, ret);

        if (strcmp(option, "new") == 0 || strcmp(option, "del") == 0){
                ret = storage_license_check();
                if (ret)
                        GOTO(err_ret, ret);
        }

        (void) srandom((uint32_t)getpid());

        /* Task distribute */
        switch (mode) {
        case MODE_TARGET:
                op = ytoken_match(target_op_tokens,
                                  ytoken_size(target_op_tokens),
                                  option);
                if (op == -1) {
                        fprintf(stderr, "invalid argument: `%s'\n", option);
                        EXIT(EINVAL);
                }

                switch (op) {
                case OP_NEW:
                case OP_DEL:
                        if (!tname) {
                                fprintf(stderr, "--target must be specified in "
                                        "the new and del operation of target mode\n");
                                EXIT(EINVAL);
                        }

                        switch (op) {
                        case OP_NEW:
                                ret = check_target_name(ns, tname);
                                if (ret)
                                        EXIT(EINVAL);

                                ret = mode_target_new(ns, tname, NULL);
                                if (ret)
                                        GOTO(err_ret, ret);
                                break;
                        case OP_DEL:
                                ret = mode_target_del(ns, tname);
                                if (ret)
                                        GOTO(err_ret, ret);
                                break;
                        }
                        break;
                case OP_LIST:
                        ret = mode_target_list();
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                }
                break;
        case MODE_LUN:
                op = ytoken_match(lun_op_tokens,
                                  ytoken_size(lun_op_tokens),
                                  option);
                if (op == -1) {
                        fprintf(stderr, "invalid argument: `%s'\n", option);
                        EXIT(EINVAL);
                }

                switch (op) {
                case OP_NEW:
                case OP_DEL:
                        if (!strcmp(ns, def) && !lname) {
                                fprintf(stderr, "--lun must be specified in "
                                        "the new and del operation of lun mode\n");
                                EXIT(EINVAL);
                        }

                        if (!tname) {
                                fprintf(stderr, "--target must be specified in the "
                                        "lun mode\n");
                                EXIT(EINVAL);
                        }

                        switch (op) {
                        case OP_NEW:
                                if (!size) {
                                        fprintf(stderr, "--size must be specified in "
                                                "the new operation of lun mode\n");
                                        EXIT(EINVAL);
                                }

                                ret = check_target_name(ns, tname);
                                if (ret)
                                        EXIT(EINVAL);

                                if (!strcmp(ns, def)) {
                                        ret = mode_lun_new_multilayer(ns, tname, lname, alias, size);
                                        if (ret)
                                                GOTO(err_ret, ret);
                                } else {
                                        ret = mode_lun_new_monolayer(ns, tname, alias, size);
                                        if (ret)
                                                GOTO(err_ret, ret);
                                }
                                break;
                        case OP_DEL:
                                if (!strcmp(ns, def)) {
                                        ret = mode_lun_del_multilayer(ns, tname, lname);
                                        if (ret)
                                                GOTO(err_ret, ret);
                                } else {
                                        ret = mode_lun_del_monolayer(ns, tname);
                                        if (ret)
                                                GOTO(err_ret, ret);
                                }
                                break;
                        }
                        break;
                case OP_LIST:
                        if (!strcmp(ns, def) && !tname) {
                                fprintf(stderr, "--target must be specified if list the "
                                        "multilayer lun mode\n");
                                EXIT(EINVAL);
                        }

                        if (!strcmp(ns, def)) {
                                ret = mode_lun_list(ns, tname);
                                if (ret)
                                        GOTO(err_ret, ret);
                        } else {
                                ret = mode_lun_list_monolayer(ns);
                                if (ret)
                                        GOTO(err_ret, ret);
                        }
                        break;
                }
                break;
        case MODE_ALIAS:
                if (!tname || !lname) {
                        fprintf(stderr, "--target and --lun must be specified in the "
                                "alias mode\n");
                        EXIT(EINVAL);
                }

                op = ytoken_match(alias_op_tokens,
                                  ytoken_size(alias_op_tokens),
                                  option);
                if (op == -1) {
                        fprintf(stderr, "invalid argument: `%s'\n", option);
                        EXIT(EINVAL);
                }

                switch (op) {
                case OP_NEW:
                        if (!alias) {
                                fprintf(stderr, "--alias must be specified in the "
                                        "alias mode\n");
                                EXIT(EINVAL);
                        }

                        ret = mode_alias_new(ns, tname, lname, alias);
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                case OP_DEL:
                        ret = mode_alias_del(ns, tname, lname);
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                case OP_LIST:
                        ret = mode_alias_list(ns, tname, lname);
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                }
                break;
        case MODE_ACCOUNT:
                if (!tname) {
                        fprintf(stderr, "--target must be specified in the "
                                "account mode\n");
                        EXIT(EINVAL);
                }

                op = ytoken_match(account_op_tokens,
                                  ytoken_size(account_op_tokens),
                                  option);
                if (op == -1) {
                        fprintf(stderr, "invalid argument: `%s'\n", option);
                        EXIT(EINVAL);
                }

                switch (op) {
                case OP_NEW:
                        if (!user || !pass || dir == -1) {
                                fprintf(stderr, "--user, --pass and --direction "
                                        "must be specified in the new operation "
                                        "of account mode\n");
                                EXIT(EINVAL);
                        }

                        ret = mode_account_new(ns, tname, user, pass, dir);
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                case OP_DEL:
                        if (dir == -1) {
                                fprintf(stderr, "--direction must be specified "
                                        "in the del operation of account mode\n");
                                EXIT(EINVAL);
                        }

                        ret = mode_account_del(ns, tname, dir);
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                case OP_LIST:
                        ret = mode_account_list(ns, tname);
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                }
                break;
        default:
                fprintf(stderr, "BUG\n");
                EXIT(EINVAL);
        }

        return 0;
err_ret:
        EXIT(_errno(ret));
}
