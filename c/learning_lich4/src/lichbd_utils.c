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
#include "net_global.h"
#include "lich_md.h"
#include "../../storage/storage/stor_rpc.h"
#include "utils.h"
#include "option.h"
#include "dbg.h"


#define LICHBD "lichbd"
#define MAX_SUBCMD_DEPTH 8
#define CMD_NEED_NULL (1 << 0)
#define CMD_NEED_ARG (1 << 1)
#define CMD_NEED_ROOT (1 << 2)

int object_md5sum(const fileid_t *oid);
struct command {
        const char *name;
        const struct subcommand *sub;
        int (*parser)(int, const char *);
};

struct subcommand {
        const char *name;
        const char *arg;
        const char *opts;
        const char *desc;
        const struct subcommand *sub;
        unsigned long flags;
        int (*fn)(int, char **);
        const struct sd_option *options;
};

static const char program_name[] = "lichbd";
static char system_cmd[] = "image";
static bool found_system_cmd = false;

int subcmd_depth = -1;
struct subcommand *subcmd_stack[MAX_SUBCMD_DEPTH];

static const struct sd_option lichbd_options[] = {
        {'v', "verbose", false, "print more information than default", NULL},
        {'h', "help", false, "display this help and exit", NULL},
        { 0, NULL, false, NULL, NULL},
};

/*static struct pool_cmd_data {*/
/*} pool_cmd_data;*/

static struct image_cmd_data {
        int output_format;
        int mult_thread;
        size_t size;
        char protocol_name[MAX_NAME_LEN];
        char image_name[MAX_NAME_LEN];
        char path[MAX_NAME_LEN];
} image_cmd_data;

struct cmd_flag_data {
        bool all_flag;
        bool mult_thread_flag;
        bool size_flag;
        bool protocol_name_flag;
        bool image_name_flag;
        bool path_flag;
        bool output_format_flag;
        int arguments;
} cmd_flag_data = {false, false, false, false, false, false, false, 0};

static struct pool_cmd_data {
        char protocol_name[MAX_NAME_LEN];
} pool_cmd_data;

static struct snap_cmd_data {
        int all;
        int output_format;
        char protocol_name[MAX_NAME_LEN];
        char image_name[MAX_NAME_LEN];
} snap_cmd_data;

void subcommand_usage(char *cmd, char *subcmd, int status);

static int (*command_parser)(int, const char *);
static int (*command_fn)(int, char **);
static const char *command_opts;
static const char *command_arg;
static const char *command_desc;
static const struct sd_option *command_options;

static void __protocol_trans(const char *_protocol, char *path)
{
        char protocol[MAX_NAME_LEN];
        
        if (_protocol && strlen(_protocol)) {
                strcpy(protocol, _protocol);
        } else {
                strcpy(protocol, gloconf.default_protocol);
        }

        if (strcmp(protocol, "lichbd") == 0) {
                snprintf(path, MAX_NAME_LEN, "%s", gloconf.lichbd_root);
        } else if (strcmp(protocol, "nbd") == 0) {
                snprintf(path, MAX_NAME_LEN, "%s", gloconf.nbd_root);
        } else if (strcmp(protocol, "sheepdog") == 0) {
                snprintf(path, MAX_NAME_LEN, "%s", gloconf.sheepdog_root);
        } else {
                snprintf(path, MAX_NAME_LEN, "%s", protocol);
        }
}

static int __create_protocol(const char *pool)
{
        int ret;
        char tmp[MAX_NAME_LEN], protocol[MAX_NAME_LEN];

        __protocol_trans(pool, protocol);

        snprintf(tmp, MAX_NAME_LEN, "/%s", protocol);
        ret = utils_mkpool(tmp);
        if (ret) {
                if (ret == EEXIST) {
                } else
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __add_lichbd_pre(char *dist, const char *src, struct cmd_flag_data cmd)
{
        char tmp[MAX_NAME_LEN], tmp1[MAX_NAME_LEN];

        if (cmd.protocol_name_flag) {
                __protocol_trans(image_cmd_data.protocol_name, tmp1);
        } else {
                __protocol_trans(gloconf.default_protocol, tmp1);
        }
        
        sprintf(tmp, "/%s/", tmp1);
        strcat(tmp, src);
        strcpy(dist, tmp);

        return 0;
}

static int __add_lichbd_pre_for_pool(char *path, const char *protocol, const char *pool)
{
        char tmp[MAX_NAME_LEN];

        __protocol_trans(protocol, tmp);
        
        if (pool) {
                snprintf(path, MAX_NAME_LEN, "/%s/%s", tmp, pool);
        } else {
                snprintf(path, MAX_NAME_LEN, "/%s", tmp);
        }

        DINFO("create %s\n", path);
        
        return 0;
}

static int __add_lichbd_pre_for_image(char *path, const char *protocol, const char* image)
{
        char tmp[MAX_NAME_LEN];

        __protocol_trans(protocol, tmp);
        snprintf(path, MAX_NAME_LEN, "/%s/%s", tmp, image);

        DINFO("create %s\n", path);
        
        return 0;
}

static int __is_valid_pool(const char *str)
{
        if (strlen(str) == 0)
                return 0;

        if (str[0] == '/' || str[strlen(str) - 1] == '/') {
                return 0;
        }

        return 1;
}

static int __is_valid_image(const char *str)
{
        if (strlen(str) == 0)
                return 0;

        if (str[0] == '/' || str[strlen(str) - 1] == '/') {
                return 0;
        }

        return 1;
}

static int __stor_mkpool(int argc, char **argv)
{
        int ret;
        const char *protocol = NULL, *pool;
        int arguments = 0;
        char path[MAX_NAME_LEN];

        arguments = cmd_flag_data.arguments * 2 + 2 + 1;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                protocol = image_cmd_data.protocol_name;
                pool = argv[argc - 1];
        }

        if (!__is_valid_pool(pool)) {
                ret = EINVAL;
                fprintf(stderr, "pool was invalid\n");
                GOTO(err_ret, ret);
        }

        ret = __create_protocol(protocol);
        if (ret)
                GOTO(err_ret, ret);
        
        __add_lichbd_pre_for_pool(path, protocol, pool);

        ret = utils_mkpool(path);
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __stor_rmpool(int argc, char **argv)
{
        int ret;
        const char *protocol = NULL, *pool = NULL;
        int arguments = 0;
        char path[MAX_NAME_LEN];

        arguments = cmd_flag_data.arguments * 2 + 2 + 1;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                protocol = image_cmd_data.protocol_name;
                pool = argv[argc - 1];
        }

        if (!__is_valid_pool(pool)) {
                ret = EINVAL;
                fprintf(stderr, "pool was invalid\n");
                GOTO(err_ret, ret);
        }

        __add_lichbd_pre_for_pool(path, protocol, pool);

        ret = utils_rmpool(path);
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __stor_lspools(int argc, char **argv)
{
        int ret;
        const char *protocol = NULL, *pool = NULL;
        int arguments = 0;
        char path[MAX_NAME_LEN];

        arguments = cmd_flag_data.arguments * 2 + 2;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments + 1) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                protocol = image_cmd_data.protocol_name;
                if (argc % 2)
                        pool = argv[argc - 1];
                else
                        pool = NULL;
        }

        __add_lichbd_pre_for_pool(path, protocol, pool);
        
        ret = utils_list(path, 1);
        if (ret) {
                if (ret != ENOENT){
                        GOTO(err_ret, ret);
                }
        }

        return 0;
err_ret:
        return ret;
}

static int __stor_create(int argc, char **argv)
{
        int ret, arguments = 0;
        char image[MAX_NAME_LEN];
        char *_image = NULL, *protocol = NULL;

        if (cmd_flag_data.image_name_flag)
                arguments = cmd_flag_data.arguments * 2 + 2;
        else
                arguments = cmd_flag_data.arguments * 2 + 2 + 1;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                if (cmd_flag_data.image_name_flag)
                        _image = image_cmd_data.image_name;
                else
                        _image = argv[argc - 1];
        }

        protocol = image_cmd_data.protocol_name;
        if (!__is_valid_image(_image)) {
                ret = EINVAL;
                fprintf(stderr, "image: %s was invalid\n", _image);
                goto err_ret;
        }

        __add_lichbd_pre_for_image(image, protocol, _image);

        ret = __create_protocol(protocol);
        if (ret)
                GOTO(err_ret, ret);
        
        ret = utils_touch(image);
        if (ret)
                GOTO(err_ret, ret);

        ret = utils_truncate(image, image_cmd_data.size);
        if (ret)
                GOTO(err_ret, ret);

        printf("create %s ok\n", _image);

        return 0;
err_ret:
        return ret;
}

static int __stor_rm(int argc, char **argv)
{
        int ret, arguments = 0;
        char image[MAX_NAME_LEN];
        char *_image = NULL, *protocol = NULL;

        if (cmd_flag_data.image_name_flag)
                arguments = cmd_flag_data.arguments * 2 + 2;
        else
                arguments = cmd_flag_data.arguments * 2 + 2 + 1;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                if (cmd_flag_data.image_name_flag)
                        _image = image_cmd_data.image_name;
                else
                        _image = argv[argc - 1];
        }

        protocol = image_cmd_data.protocol_name;
        if (!__is_valid_image(_image)) {
                ret = EINVAL;
                fprintf(stderr, "image: %s was invalid\n", _image);
                GOTO(err_ret, ret);
        }

        __add_lichbd_pre_for_image(image, protocol, _image);

        ret = utils_rmvol(image);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int __stor_ls(int argc, char **argv)
{
        int ret, arguments = 0;
        const char *protocol = NULL, *pool = NULL;
        char path[MAX_NAME_LEN];
        
        if (argc % 2) {
                if (cmd_flag_data.protocol_name_flag)
                        arguments = cmd_flag_data.arguments * 2 + 2 + 1;
                else
                        arguments = cmd_flag_data.arguments * 2 + 2 + 1;
        } else {
                arguments = cmd_flag_data.arguments * 2 + 2;
        }

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                protocol = image_cmd_data.protocol_name;
                if (argc % 2)
                        pool = argv[argc - 1];
                else
                        pool = NULL;
        }

        __add_lichbd_pre_for_pool(path, protocol, pool);

        ret = utils_list(path, 0);
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
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
               "chknum : %u\n"
               "allocated : %u\n"
               "localized : %u\n"
               "rollback : %u\n",
               network_rname(&chkinfo->diskid[0].id),
               filestat.chknum,
               filestat.chknum - filestat.sparse,
               filestat.localized,
               filestat.rollback);

        return 0;
err_ret:
        return ret;
}

static int __stor_info(int argc, char **argv)
{
        int ret, arguments = 0;
        char image[MAX_NAME_LEN];
        char *_image = NULL, *protocol = NULL, *str = NULL;

        if (cmd_flag_data.image_name_flag)
                arguments = cmd_flag_data.arguments * 2 + 2;
        else
                arguments = cmd_flag_data.arguments * 2 + 2 + 1;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                if (cmd_flag_data.image_name_flag)
                        _image = image_cmd_data.image_name;
                else
                        _image = argv[argc - 1];

                str = strchr(_image, '@');
                if (str) {
                        *str = '\0';
                }
        }

        protocol = image_cmd_data.protocol_name;
        if (!__is_valid_image(_image)) {
                ret = EINVAL;
                fprintf(stderr, "image: %s was invalid\n", _image);
                GOTO(err_ret, ret);
        }

        __add_lichbd_pre_for_image(image, protocol, _image);

        ret = __lich_stat(image, image_cmd_data.output_format);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int __stor_resize(int argc, char **argv)
{
        int ret, arguments = 0;
        char image[MAX_NAME_LEN];
        char *_image = NULL, *protocol = NULL;

        if (cmd_flag_data.image_name_flag)
                arguments = cmd_flag_data.arguments * 2 + 2;
        else
                arguments = cmd_flag_data.arguments * 2 + 2 + 1;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                if (cmd_flag_data.image_name_flag)
                        _image = image_cmd_data.image_name;
                else
                        _image = argv[argc - 1];
        }

        protocol = image_cmd_data.protocol_name;
        if (!__is_valid_image(_image)) {
                ret = EINVAL;
                fprintf(stderr, "image: %s was invalid\n", _image);
                GOTO(err_ret, ret);
        }

        __add_lichbd_pre_for_image(image, protocol, _image);

        ret = utils_truncate(image, image_cmd_data.size);
        if (ret)
                GOTO(err_ret, ret);

        printf("resize %s ok\n", _image);

        return 0;
err_ret:
        return ret;
}

static int __stor_copy(int argc, char **argv)
{
        int ret, arguments = 0;
        char from[MAX_NAME_LEN], to[MAX_NAME_LEN];
        char *_from = NULL, *_to = NULL, *protocol = NULL;

        arguments = cmd_flag_data.arguments * 2 + 2 + 2;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                _from = argv[argc - 2];
                _to = argv[argc - 1];
        }

        protocol = image_cmd_data.protocol_name;
        if (!__is_valid_image(_from)) {
                ret = EINVAL;
                fprintf(stderr, "image: %s was invalid\n", _from);
                GOTO(err_ret, ret);
        }
        __add_lichbd_pre_for_image(from, protocol, _from);

        if (!__is_valid_image(_to)) {
                ret = EINVAL;
                fprintf(stderr, "image: %s was invalid\n", _to);
                GOTO(err_ret, ret);
        }
        __add_lichbd_pre_for_image(to, protocol, _to);

        ret = stor_copy(from, to, image_cmd_data.mult_thread);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int __stor_export(int argc, char **argv)
{
        int ret, arguments;
        char from[MAX_NAME_LEN], to[MAX_NAME_LEN];
        char *protocol = NULL, *_from = NULL, *_to = NULL;

        if (cmd_flag_data.image_name_flag)
                arguments = cmd_flag_data.arguments * 2 + 2 + 1;
        else
                arguments = cmd_flag_data.arguments * 2 + 2 + 2;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                if (cmd_flag_data.image_name_flag)
                        _from = image_cmd_data.image_name;
                else
                        _from = argv[argc - 2];
        }

        _to = argv[argc - 1];
        protocol = image_cmd_data.protocol_name;
        if (!__is_valid_image(_from)) {
                ret = EINVAL;
                fprintf(stderr, "image: %s was invalid\n", _from);
                GOTO(err_ret, ret);
        }
        __add_lichbd_pre_for_image(from, protocol, _from);

        if (_to[0] == '/' || _to[0] == '.') {
                strcpy(to, ":");
                strcat(to, _to);
                ret = stor_copy(from, to, image_cmd_data.mult_thread);
                if (ret)
                        GOTO(err_ret, ret);
        } else if (_to[0] == '-') {
                ret = utils_cat(from);
                if (ret)
                        GOTO(err_ret, ret);
        } else {
                ret = EINVAL;
                fprintf(stderr, "path is invalid\n");
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __stor_import(int argc, char **argv)
{
        int ret, arguments = 0;
        char from[MAX_NAME_LEN], to[MAX_NAME_LEN];
        char *protocol = NULL, *_from = NULL, *_to = NULL;

        if (cmd_flag_data.image_name_flag)
                arguments = cmd_flag_data.arguments * 2 + 2 + 1;
        else
                arguments = cmd_flag_data.arguments * 2 + 2 + 2;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                if (cmd_flag_data.image_name_flag)
                        _to = image_cmd_data.image_name;
                else
                        _to = argv[argc - 1];
        }

        _from = argv[argc - 2];
        protocol = image_cmd_data.protocol_name;
        if (!__is_valid_image(_to)) {
                ret = EINVAL;
                fprintf(stderr, "image: %s was invalid\n", _from);
                GOTO(err_ret, ret);
        }
        __add_lichbd_pre_for_image(to, protocol, _to);

        ret = __create_protocol(protocol);
        if (ret)
                GOTO(err_ret, ret);
        
        if (_from[0] == '/' || _from[0] == '.' || _from[0] == '-') {
                strcpy(from, ":");
                strcat(from, _from);
        } else {
                ret = EINVAL;
                fprintf(stderr, "path is invalid\n");
                GOTO(err_ret, ret);
        }

        ret = stor_copy(from, to, image_cmd_data.mult_thread);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int __stor_mv(int argc, char **argv)
{
        int ret, arguments = 0;
        char from[MAX_NAME_LEN], to[MAX_NAME_LEN];
        char *_from = NULL, *_to = NULL, *protocol = NULL;

        arguments = cmd_flag_data.arguments * 2 + 2 + 2;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                _from = argv[argc - 2];
                _to = argv[argc - 1];
        }

        protocol = image_cmd_data.protocol_name;
        if (!__is_valid_image(_from)) {
                ret = EINVAL;
                fprintf(stderr, "image: %s was invalid\n", _from);
                GOTO(err_ret, ret);
        }
        __add_lichbd_pre_for_image(from, protocol, _from);

        if (!__is_valid_image(_to)) {
                ret = EINVAL;
                fprintf(stderr, "image: %s was invalid\n", _to);
                GOTO(err_ret, ret);
        }
        __add_lichbd_pre_for_image(to, protocol, _to);

        ret = utils_rename(from, to);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

#if 1
static int __is_valid_snappath(const char *snappath)
{
        int count = 0;
        char tmp[MAX_NAME_LEN];
        char *list[2];

        count = 2;
        strcpy(tmp, snappath);
        _str_split(tmp, '@', list, &count); 
        if (count != 2) {
                return 0;
        }

        return __is_valid_image(list[0]);
}

static int __stor_snap_create(int argc, char **argv)
{
        int ret, arguments = 0;
        char snappath[MAX_NAME_LEN];
        char *_snappath = NULL, *protocol = NULL;

        arguments = cmd_flag_data.arguments * 2 + 2 + 2;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                _snappath = argv[argc - 1];
        }

        protocol = snap_cmd_data.protocol_name;
        if (!__is_valid_snappath(_snappath)) {
                ret = EINVAL;
                fprintf(stderr, "snap path was invalid\n");
                GOTO(err_ret, ret);
        }

        __add_lichbd_pre_for_image(snappath, protocol, _snappath);
        ret = utils_snapshot(snappath);
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __stor_snap_ls(int argc, char **argv)
{
        int ret, all, arguments = 0;
        char image[MAX_NAME_LEN];
        char *_image = NULL, *protocol = NULL;

        if (cmd_flag_data.image_name_flag)
                arguments = cmd_flag_data.arguments * 2 + 2 + 1;
        else
                arguments = cmd_flag_data.arguments * 2 + 2 + 2;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                if (cmd_flag_data.image_name_flag)
                        _image = snap_cmd_data.image_name;
                else
                        _image = argv[argc - 1];
        }

        protocol = snap_cmd_data.protocol_name;
        if (!__is_valid_image(_image)) {
                ret = EINVAL;
                fprintf(stderr, "snap path was invalid\n");
                GOTO(err_ret, ret);
        }

        __add_lichbd_pre_for_image(image, protocol, _image);

        all = snap_cmd_data.all;
        ret = utils_snapshot_list(image, all, snap_cmd_data.output_format);
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __stor_snap_rm(int argc, char **argv)
{
        int ret, arguments = 0;
        char snappath[MAX_NAME_LEN];
        char *_snappath = NULL, *protocol = NULL;

        arguments = cmd_flag_data.arguments * 2 + 2 + 2;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                _snappath = argv[argc - 1];
        }

        protocol = snap_cmd_data.protocol_name;
        if (!__is_valid_snappath(_snappath)) {
                ret = EINVAL;
                fprintf(stderr, "snap path was invalid\n");
                GOTO(err_ret, ret);
        }

        __add_lichbd_pre_for_image(snappath, protocol, _snappath);
        ret = utils_snapshot_remove(snappath);
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __stor_snap_rollback(int argc, char **argv)
{
        int ret, arguments = 0;
        char snappath[MAX_NAME_LEN];
        char *_snappath = NULL, *protocol = NULL;

        arguments = cmd_flag_data.arguments * 2 + 2 + 2;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                _snappath = argv[argc - 1];
        }

        protocol = snap_cmd_data.protocol_name;
        if (!__is_valid_snappath(_snappath)) {
                ret = EINVAL;
                fprintf(stderr, "snap path was invalid\n");
                GOTO(err_ret, ret);
        }

        __add_lichbd_pre_for_image(snappath, protocol, _snappath);
        ret = utils_snapshot_rollback(snappath);
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __stor_snap_protect(int argc, char **argv)
{
        int ret, arguments = 0;
        char snappath[MAX_NAME_LEN];
        char *_snappath = NULL, *protocol = NULL;

        arguments = cmd_flag_data.arguments * 2 + 2 + 2;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                _snappath = argv[argc - 1];
        }

        protocol = snap_cmd_data.protocol_name;
        if (!__is_valid_snappath(_snappath)) {
                ret = EINVAL;
                fprintf(stderr, "snap path was invalid\n");
                GOTO(err_ret, ret);
        }

        __add_lichbd_pre_for_image(snappath, protocol, _snappath);
        ret = utils_snapshot_protect(snappath);
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __stor_snap_unprotect(int argc, char **argv)
{
        int ret, arguments = 0;
        char snappath[MAX_NAME_LEN];
        char *_snappath = NULL, *protocol = NULL;

        arguments = cmd_flag_data.arguments * 2 + 2 + 2;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                _snappath = argv[argc - 1];
        }

        protocol = snap_cmd_data.protocol_name;
        if (!__is_valid_snappath(_snappath)) {
                ret = EINVAL;
                fprintf(stderr, "snap path was invalid\n");
                GOTO(err_ret, ret);
        }

        __add_lichbd_pre_for_image(snappath, protocol, _snappath);
        ret = utils_snapshot_unprotect(snappath);
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __stor_snap_clone(int argc, char **argv)
{
        int ret, arguments = 0;
        char snappath[MAX_NAME_LEN], to[MAX_NAME_LEN];
        char *_snappath = NULL, *_to = NULL;

        arguments = cmd_flag_data.arguments * 2 + 2 + 2;

        if (argc < arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too few arguments\n");
                GOTO(err_ret, ret);
        } else if (argc > arguments) {
                ret = EINVAL;
                fprintf(stderr, "lichbd: too more arguments\n");
                GOTO(err_ret, ret);
        } else {
                _snappath = argv[argc - 2];
                _to = argv[argc - 1];
        }

        if (!__is_valid_snappath(_snappath)) {
                ret = EINVAL;
                fprintf(stderr, "snap path was invalid\n");
        }

        __add_lichbd_pre(snappath, _snappath, cmd_flag_data);

        if (!__is_valid_image(_to)) {
                ret = EINVAL;
                fprintf(stderr, "_image was invalid\n");
                GOTO(err_ret, ret);
        }
        __add_lichbd_pre(to, _to, cmd_flag_data);

        ret = utils_snapshot_clone(snappath, to);
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}
#endif

#if 0
static int __stor_snap_copy(const char *_snappath, const char *to)
{
        int ret;
        char snappath[MAX_NAME_LEN];

        if (!__is_valid_snappath(_snappath)) {
                ret = EINVAL;
                fprintf(stderr, "snap path was invalid\n");
                GOTO(err_ret, ret);
        }
        __add_lichbd_pre(snappath, _snappath);

        ret = utils_snapshot_copy(snappath, to);
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}
#endif

static void usage(const struct command *commands, int status)
{
        int i;
        const struct subcommand *s;
        char name[64];
        char text[MAX_NAME_LEN];

        if (status) {
                fprintf(stderr, "Try '%s help' for more information.\n", program_name);
        } else {
                printf("lichbd (version %s)\n",
                                PACKAGE_VERSION);
                printf("Usage: %s [OPTIONS] <cmd> ...\n", program_name);
                printf("\nAvailable commands:\n");
                for (i = 0; commands[i].name; i++) {
                        for (s = commands[i].sub; s->name; s++) {
                                if (!strcmp(commands[i].name, system_cmd)) {
                                        snprintf(name, sizeof(name), "%s", s->name);
                                } else {
                                        snprintf(name, sizeof(name), "%s %s",
                                                commands[i].name, s->name);
                                }
                                sprintf(text, "%s %s", name, s->arg);
                                if (strlen(text) >= 45) {
                                        printf("  %s\n", text);
                                        printf("  %-45s%s\n", "", s->desc);
                                } else {
                                        printf("  %-45s%s\n", text, s->desc);
                                }
                        }
                }
                printf("\n");
                printf("supported protocol: iscsi, lichbd, nbd\n");
                printf("For more information, run "
                                "'%s <command> <subcommand> help'.\n", program_name);
        }
        exit(status);
}

static struct sd_option pool_options[] = {
        {'p', "pool", true, "source pool name", NULL},
        { 0, NULL, false, NULL, NULL},
};

static struct sd_option image_options[] = {
        {'S', "single", false, "do not use multi thread", "do not use mult thread"},
        {'s', "size", true, "size of image for create and resize", NULL},
        {'p', "pool", true, "source pool name", NULL},
        {'P', "path", true, "path name for import/export", NULL},
        {'i', "image", true, "image name", NULL},
        {'I', "image-format", true, "image format:1, 2", NULL},
        {'f', "format", true, "output format, such as json", NULL},
        { 0, NULL, false, NULL, NULL},
};

static struct sd_option snap_options[] = {
        {'a', "all", false, "show all snap, include deleted", "show all snap, include deleted"},
        {'p', "pool", true, "source pool name", NULL},
        {'f', "format", true, "output format, such as json", NULL},
        { 0, NULL, false, NULL, NULL},
};

/*static struct sd_option snap_options[] = {*/
/*{ 0, NULL, false, NULL },*/
/*};*/

static struct subcommand pool_cmd[] = {
        {"mkpool", "<pool>", "p", "make an pool",
                NULL, CMD_NEED_ARG,
                __stor_mkpool, pool_options},
        {"rmpool", "<pool>", "p", "rm an pool",
                NULL, CMD_NEED_ARG,
                __stor_rmpool, pool_options},
        {"lspool", "", "p", "list all pools",
                NULL, CMD_NEED_NULL,
                __stor_lspools, pool_options},
        {NULL, NULL, NULL, NULL, NULL, CMD_NEED_NULL, NULL, NULL},
};

const char copy_help[] = "<src> <dist>\n\n \
example of <src> <dist>: \n \
        <pool/src-image>       <pool/dist-image>\n \
        </path/to/local-file>  <pool/dist-image>\n \
        <pool/src-image>       </path/to/local-file\n \
";

static struct subcommand image_cmd[] = {
        {"mkpool", "<pool> [-p protocol]", "p", "create pool <pool>",
                NULL, CMD_NEED_ARG,
                __stor_mkpool, pool_options},
        {"rmpool", "<pool> [-p protocol]", "p", "remove pool <pool>",
                NULL, CMD_NEED_ARG,
                __stor_rmpool, pool_options},
        {"lspools", "[pool] [-p protocol]", "p", "list pools",
                NULL, CMD_NEED_NULL,
                __stor_lspools, pool_options},
        {"create", "<image> --size <M> [-p protocol]", "psSI", "create an empty image,unit(k|K, |m|M, g|G)",
                NULL, CMD_NEED_ARG,
                __stor_create, image_options},
        {"rm", "<image> [-p protocol]", "p", "delete an image",
                NULL, CMD_NEED_ARG,
                __stor_rm, image_options},
        {"ls", "[pool] [-p protocol]", "p", "list lichbd images",
                NULL, CMD_NEED_ARG,
                __stor_ls, image_options},
        {"info", "<image> [-p protocol]", "pf", "show information about image size, date, etc",
                NULL, CMD_NEED_ARG,
                __stor_info, image_options},
        {"resize", "<image> --size <M> [-p protocol]", "ps", "resize the image,unit(k|K, |m|M, g|G)",
                NULL, CMD_NEED_ARG,
                __stor_resize, image_options},
        {"copy", "<src> <dest> [-p protocol]", "pS", "copy src image to dest",
                NULL, CMD_NEED_ARG,
                __stor_copy, image_options},
        {"export", "<image> <path> [-p protocol]", "p", "export image to file. '-' for stdout",
                NULL, CMD_NEED_ARG,
                __stor_export, image_options},
        {"import", "<path> <image> [-p protocol]", "pI", "import image from file. '-' for stdin",
                NULL, CMD_NEED_ARG,
                __stor_import, image_options},
        {"mv", "<src> <dest> [-p protocol]", "p", "move src image to dest",
                NULL, CMD_NEED_ARG,
                __stor_mv, image_options},
        {"rename", "<src> <dest> [-p protocol]", "p", "rename src image to dest",
                NULL, CMD_NEED_ARG,
                __stor_mv, image_options},
        {"clone", "<pool/image>@<snap> <pool/dest-image> [-p protocol]", "p", "clone a snapshot into a COW",
                NULL, CMD_NEED_ARG,
                __stor_snap_clone, image_options},
        {NULL, NULL, NULL, NULL, NULL, CMD_NEED_NULL, NULL, NULL},
};

static struct subcommand snap_cmd[] = {
        {"create", "<image>@<snap> [-p protocol]", "p", "create a snapshot",
                NULL, CMD_NEED_ARG,
                __stor_snap_create, snap_options},
        {"ls", "<image> [-p protocol]", "pf", "dump list of image snapshots",
                NULL, CMD_NEED_ARG,
                __stor_snap_ls, snap_options},
        {"remove", "<image>@<snap> [-p protocol]", "p", "deletes a snapshot",
                NULL, CMD_NEED_ARG,
                __stor_snap_rm, snap_options},
        {"rollback", "<image>@<snap> [-p protocol]", "p", "rollback image to snapshot",
                NULL, CMD_NEED_ARG,
                __stor_snap_rollback, snap_options},
        {"protect", "<image>@<snap> [-p protocol]", "p", "prevent a snapshot from being deleted",
                NULL, CMD_NEED_ARG,
                __stor_snap_protect, snap_options},
        {"unprotect", "<image>@<snap> [-p protocol]", "p", "allow a snapshot to be deleted",
                NULL, CMD_NEED_ARG,
                __stor_snap_unprotect, snap_options},
        {NULL, NULL, NULL, NULL, NULL, CMD_NEED_NULL, NULL, NULL},
};

static int pool_parser(int ch, const char *opt)
{
        DINFO("ch : %d, opt: %s\n", ch, opt);
        switch (ch) {
                case 'p':
                        strcpy(pool_cmd_data.protocol_name, optarg);
                        cmd_flag_data.arguments += 1;
                        cmd_flag_data.protocol_name_flag = true;
                        break;
                default:
                        break;
        }

        return 0;
}

static int image_parser(int ch, const char *opt)
{
        int ret;
        DINFO("ch : %d, opt: %s\n", ch, opt);

        switch (ch) {
                case 'f':
                        if (!strcmp("json", optarg)) {
                                image_cmd_data.output_format = 1;
                        } else {
                                image_cmd_data.output_format = 0;
                        }
                        cmd_flag_data.arguments += 1;
                        cmd_flag_data.output_format_flag = true;
                        break;
                case 'i':
                        strcpy(image_cmd_data.image_name, optarg);
                        cmd_flag_data.arguments += 1;
                        cmd_flag_data.image_name_flag = true;
                        break;
                case 'I':
                        cmd_flag_data.arguments += 1;
                        break;
                case 'S':
                        image_cmd_data.mult_thread = 0;
                        cmd_flag_data.mult_thread_flag = true;
                        break;
                case 'p':
                        strcpy(image_cmd_data.protocol_name, optarg);
                        cmd_flag_data.arguments += 1;
                        cmd_flag_data.protocol_name_flag = true;
                        break;
                case 'P':
                        strcpy(image_cmd_data.path, optarg);
                        cmd_flag_data.arguments += 1;
                        cmd_flag_data.path_flag = true;
                        break;
                case 's':
                        ret = utils_get_size(optarg, &image_cmd_data.size);
                        if (ret)
                                goto err_ret;
                        cmd_flag_data.arguments += 1;
                        cmd_flag_data.path_flag = true;
                        break;
                default:
                        break;
        }

        return 0;
err_ret:
        return ret;
}

static int snap_parser(int ch, const char *opt)
{
        DINFO("ch : %d, opt: %s\n", ch, opt);
        switch (ch) {
                case 'a':
                        snap_cmd_data.all = 1;
                        cmd_flag_data.arguments += 1;
                        cmd_flag_data.all_flag = true;
                        break;
                case 'f':
                        if (!strcmp("json", optarg)) {
                                snap_cmd_data.output_format = 1;
                        } else {
                                snap_cmd_data.output_format = 0;
                        }
                        cmd_flag_data.arguments += 1;
                        cmd_flag_data.output_format_flag = true;
                        break;
                case 'i':
                        strcpy(snap_cmd_data.image_name, optarg);
                        cmd_flag_data.arguments += 1;
                        cmd_flag_data.image_name_flag = true;
                        break;
                case 'p':
                        strcpy(snap_cmd_data.protocol_name, optarg);
                        cmd_flag_data.arguments += 1;
                        cmd_flag_data.protocol_name_flag = true;
                        break;
                default:
                        break;
        }

        return 0;
}

struct command pool_command = {
        "image",
        pool_cmd,
        pool_parser
};

struct command image_command = {
        "image",
        image_cmd,
        image_parser
};

struct command snap_command = {
        "snap",
        snap_cmd,
        snap_parser
};

static const struct sd_option *find_opt(int ch)
{
        const struct sd_option *opt;

        /* search for common options */
        sd_for_each_option(opt, lichbd_options) {
                if (opt->ch == ch)
                        return opt;
        }

        /* search for self options */
        if (command_options) {
                sd_for_each_option(opt, command_options) {
                        if (opt->ch == ch)
                                return opt;
                }
        }

        fprintf(stderr, "Internal error\n");
        exit(1);
}

static void init_commands(const struct command **commands)
{
        int ret;
        static struct command *cmds;
        struct command command_list[] = {
                image_command,
                snap_command,
                {NULL, NULL, NULL},
        };

        if (!cmds) {
                ret = ymalloc((void **)&cmds, sizeof(command_list));
                if (ret) {
                        YASSERT(0);
                }
                memcpy(cmds, command_list, sizeof(command_list));
        }

        *commands = cmds;
        return;
}

static const struct subcommand *find_subcmd(const char *cmd, const char *subcmd)
{
        int i, j;
        const struct command *commands;
        const struct subcommand *sub;

        init_commands(&commands);

        for (i = 0; commands[i].name; i++) {
                if (!strcmp(commands[i].name, cmd)) {
                        sub = commands[i].sub;
                        for (j = 0; sub[j].name; j++) {
                                if (!strcmp(sub[j].name, subcmd))
                                        return &sub[j];
                        }
                }
        }

        return NULL;
}

static unsigned long setup_commands(const struct command *commands,
                const char *cmd, const char *subcmd)
{
        int i;
        bool found = false;
        const struct subcommand *s;
        unsigned long flags = 0;

        for (i = 0; commands[i].name; i++) {
                if (!strcmp(commands[i].name, cmd)) {
                        found = true;
                        if (commands[i].parser)
                                command_parser = commands[i].parser;
                        break;
                }
        }

        if (!found) {
                for (i = 0; commands[i].name; i++) {
                        if (!strcmp(commands[i].name, system_cmd)) {
                                break;
                        }
                }

                for (s = commands[i].sub; s->name; s++) {
                        if (!strcmp(s->name, cmd)) {
                                found = true;
                                found_system_cmd = true;
                                if (commands[i].parser)
                                        command_parser = commands[i].parser;
                                break;
                        }
                }
        }

        if (!found) {
                if (cmd && strcmp(cmd, "help") && strcmp(cmd, "--help") &&
                                strcmp(cmd, "-h")) {
                        fprintf(stderr, "Invalid command '%s'\n", cmd);
                        usage(commands, EINVAL);
                }
                usage(commands, 0);
        }

        for (s = commands[i].sub; (!found_system_cmd?subcmd:cmd) && s->name; s++) {
                if (!strcmp(s->name, (!found_system_cmd?subcmd:cmd))) {
                        command_fn = s->fn;
                        command_opts = s->opts;
                        command_arg = s->arg;
                        command_desc = s->desc;
                        command_options = s->options;
                        flags = s->flags;
                        break;
                }
        }

        if (!command_fn) {
                if (found_system_cmd) {
                        usage(commands, 0);
                }

                if (subcmd && strcmp(subcmd, "help") &&
                                strcmp(subcmd, "--help") && strcmp(subcmd, "-h"))
                        fprintf(stderr, "Invalid command '%s %s'\n", cmd, subcmd);

                fprintf(stderr, "Available %s commands:\n", cmd);
                for (s = commands[i].sub; s->name; s++)
                        fprintf(stderr, "  %s %s\n", cmd, s->name);
                exit(EINVAL);
        }

        return flags;
}

void subcommand_usage(char *cmd, char *subcmd, int status)
{
        int i, n, len = strlen(command_opts);
        const struct sd_option *sd_opt;
        const struct subcommand *sub, *subsub;
        char name[64];
        char text[MAX_NAME_LEN];

        printf("Usage: %s %s %s", program_name, found_system_cmd?"":cmd, subcmd);

        if (0 <= subcmd_depth) {
                for (i = 0; i < subcmd_depth + 1; i++)
                        printf(" %s", subcmd_stack[i]->name);

                subsub = subcmd_stack[i - 1]->sub;
        } else {
                sub = find_subcmd(cmd, subcmd);
                subsub = sub->sub;
        }

        if (subsub) {
                n = 0;
                while (subsub[n].name)
                        n++;
                if (n == 1)
                        printf(" %s", subsub[0].name);
                else if (n > 1) {
                        printf(" {%s", subsub[0].name);
                        for (i = 1; i < n; i++)
                                printf("|%s", subsub[i].name);
                        printf("}");
                }
        }

        for (i = 0; i < len; i++) {
                sd_opt = find_opt(command_opts[i]);
                if (sd_opt->has_arg) {
                        if (sd_opt->ch != 's')
                                printf(" [-%c %s]", sd_opt->ch, sd_opt->name);
                }
                else
                        printf(" [-%c]", sd_opt->ch);
        }
        if (command_arg)
                printf(" %s", command_arg);

        printf("\n");
        if (subsub) {
                printf("Available subcommands:\n");
                for (i = 0; subsub[i].name; i++) {
                        sprintf(text, "%s %s", subsub[i].name, subsub[i].arg);
                        if (strlen(text) >= 45) {
                                printf("  %s\n", text);
                                printf("  %-45s%s\n", "", subsub[i].desc);
                        } else {
                                printf("  %-45s%s\n", text, subsub[i].desc);
                        }
                }

        }

        for (i = 0; i < len; i++) {
                if (i == 0)
                        printf("Options:\n");
                sd_opt = find_opt(command_opts[i]);
                snprintf(name, sizeof(name), "-%c, --%s",
                                sd_opt->ch, sd_opt->name);
                printf("  %-24s%s\n", name, sd_opt->desc);
        }

        exit(status);
}

static const struct sd_option *build_sd_options(const char *opts)
{
        static struct sd_option sd_opts[256], *p;
        int i, len = strlen(opts);

        p = sd_opts;
        for (i = 0; i < len; i++)
                *p++ = *find_opt(opts[i]);
        memset(p, 0, sizeof(struct sd_option));

        return sd_opts;
}

static int format_args(int argc, char *argv[], char *new[])
{
        int i, c = 0, c1 = 0;
        char *new1[256];

        for (i = 0; i < argc; i++) {
                if ('-' == argv[i][0]) {
                        new1[c1] = (char *)malloc(256);
                        strcpy(new1[c1++], argv[i++]);
                        if (NULL != argv[i]) {
                                new1[c1] = (char *)malloc(256);
                                strcpy(new1[c1++], argv[i]);
                        }
                } else {
                        new[c] = (char*)malloc(256);
                        strcpy(new[c++], argv[i]);
                }
        }

        for (i = 0; i < c1; i++) {
                new[i + c] = (char*)malloc(256);
                strcpy(new[i + c], new1[i]);
        }

        return 0;
}

int main(int argc, char *argv[])
{
        int ch, longindex, ret;
        unsigned long flags;
        struct option *long_options;
        const struct command *commands;
        const char *short_options;
        const struct sd_option *sd_opts;
        bool verbose;
        char argument1[256], argument2[256];
        char *argvnew[256];

        dbg_info(0);

        init_commands(&commands);

        if (argc == 1) {
                usage(commands, EINVAL);
                exit(EINVAL);
        }

        format_args(argc, argv, argvnew);

        if (argc > 2) {
                strcpy(argument1, argvnew[1]);
                strcpy(argument2, argvnew[2]);
                flags = setup_commands(commands, argvnew[1], argvnew[2]);
        } else {
                strcpy(argument1, argvnew[1]);
                flags = setup_commands(commands, argvnew[1], NULL);
        }

        optind = 3?!found_system_cmd:2;

        sd_opts = build_sd_options(command_opts);
        long_options = build_long_options(sd_opts);
        short_options = build_short_options(sd_opts);

        while ((ch = getopt_long(argc, argv, short_options, long_options,
                                        &longindex)) >= 0) {

                switch (ch) {
                        case 'v':
                                verbose = true;
                                break;
                        case 'h':
                                subcommand_usage(argv[1], argv[2], 0);
                                break;
                        case '?':
                                usage(commands, EINVAL);
                                break;
                        default:
                                if (command_parser) {
                                        ret = command_parser(ch, optarg);
                                        if (ret)
                                                GOTO(err_ret, ret);
                                }
                                else
                                        usage(commands, EINVAL);
                                break;
                }
        }

        (void) verbose;
        if (flags & CMD_NEED_ARG && argc == optind) {
                if (found_system_cmd) {
                        subcommand_usage(system_cmd, argument1, EINVAL);
                } else {
                        subcommand_usage(argument1, argument2, EINVAL);
                }
        }

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

        int i;
        for (i=0; i < argc; ++i) {
                printf("arg: %s\n", argv[i]);
        }

        ret = command_fn(argc, argv);
        if (ret) {
                if (ret == EINVAL) {
                        if (found_system_cmd) {
                                subcommand_usage(system_cmd, argument1, EINVAL);
                        } else {
                                subcommand_usage(argument1, argument2, EINVAL);
                        }
                } else {
                        GOTO(err_ret, ret);
                }
        }

        return 0;
err_ret:
        EXIT(_errno(ret));
}
