#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <errno.h>
#include <termios.h>
#include <openssl/aes.h>
#include <openssl/md5.h>
#include <getopt.h>
#include <dirent.h>
#include <math.h>

#include "configure.h"
#include "sysy_lib.h"
#include "dbg.h"
#include "adt.h"
#include "storage.h"
#include "license.h"

static void usage()
{
        fprintf(stderr, "\nusage: lich.license [-vh]\n"
                "                             [-m sniffer ]\n"//[-o outfile]\n"
                "                             [-m list]\n"
                "\t-v --verbose         Show verbose message\n"
                "\t-h --help            Show this help\n"
                "\t-m --mode            sniffer: sniffer hardware info to outfile\n"
                "\t                        list: list license infomation\n"
                //"\t-o --outfile         Output file\n"
                "\n"
               );
}

static int __check_free_limit(int *free_limit)
{
        int ret;
        time_t now;
        uint32_t createtime;
	struct stat buf;
	char license[MAX_NAME_LEN];

        ret = storage_get_createtime(&createtime);
        if (ret)
                GOTO(err_ret, ret);

        now = gettime();
        *free_limit = 0;
	snprintf(license, MAX_NAME_LEN, "%s/%s", gloconf.home, "license");
	ret = stat(license, &buf);
        if (ret) {
		if (now - createtime <= FREE_LICENSE) {
			*free_limit = createtime + FREE_LICENSE;
		}
        } else {
                if (buf.st_size == 0) {
			if (now - createtime <= FREE_LICENSE) {
				*free_limit = createtime + FREE_LICENSE;
			}
                }
        }

        return 0;
err_ret:
        return ret;
}

static int __sniffer(char *path)
{
        int ret, fd=0;

        (void) path;
        //modify 'lich.license -m sniffer' output from file to terminal
        /*
        fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0200);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }
        */

        ret = license_sniffer(fd);
        if (ret)
                GOTO(err_ret, ret);

        //close(fd);

        return 0;
//err_close:
        //close(fd);
err_ret:
        return ret;
}

/**
 * function: check whether the node is online
 *           if the node is online,  return 0
 *           if the node is offline, return ENONET
 */
static int __check_node_isonline(const char* path)
{
        int ret;
        struct stat stbuf;
        char buf[MAX_NAME_LEN];

        ret = stat(path, &stbuf);
        if (ret < 0) {
                ret = errno;
                goto err_ret;
        }

	ret = _get_value(path, buf, MAX_NAME_LEN);
	if (ret < 0) {
		ret = errno;
		GOTO(err_ret, ret);
	}

	if (!strcmp(buf, "stopped\n")) {
		ret = ENONET;
		GOTO(err_ret, ret);
	}

        return 0;
err_ret:
        return ret;
}

static int __decrypt()
{
        int ret, free_limit = 0, fd = 0;
        long capacity = 0;
        time_t limit = 0;
        struct tm *tm_limit;
        char buf[MAXSIZE+1], license_path[MAX_PATH_LEN];
        char status[MAX_NAME_LEN];


        if (cluster_is_solomode()) {
                limit = -1;
                capacity = -1;
                ret = 0;
                GOTO(err_ret, ret);
        }

        snprintf(status, MAX_NAME_LEN, "%s/%s", gloconf.home, "/data/status/status");
        ret = __check_node_isonline(status);
        if (ret)
                GOTO(err_ret, ret);

        ret = __check_free_limit(&free_limit);
        if(ret)
                GOTO(err_ret, ret);

        ret = license_check(gloconf.home, &capacity, free_limit);
        if (ret)
                GOTO(err_ret, ret);

        sprintf(license_path, "%s/license", gloconf.home);
        fd = open(license_path, O_RDONLY);
        if (fd < 0) {
                ret = errno;
                if (ret == ENOENT)
                        goto err_ret;
                else
                        GOTO(err_ret, ret);
        }   

        ret = license_decrypt(fd, &limit);
        if (ret)
                GOTO(err_close, ret);

err_close:
        close(fd);

err_ret:
        if (limit == -1) {
                printf("Permanent free license.\n");
        } else if (limit < free_limit) {
                limit = free_limit;
                capacity = -1;

                printf("Free license, ");
        } else if (limit != 0)
                printf("Valid license, ");

        if (limit != -1 && limit != 0) {
                tm_limit = localtime(&limit);
                strftime(buf, MAXSIZE, "%F %T", tm_limit);
                printf("license expiration time: %s\n", buf);

                if (capacity == -1) {
                        printf("Infinite capacity.\n");
                } else {
                        printf("Permit capacity:%ldG.\n", (long)ceil(capacity/(1000.0*1000.0*1000.0)));
                }

                return 0;
        } else if (ret == 0) {
                if (capacity == -1) {
                        printf("Infinite capacity.\n");
                } else {
                        printf("Permit capacity:%ldG.\n", (long)ceil(capacity/(1000.0*1000.0*1000.0)));
                }

                return 0;
        }

        if (ret == EAGAIN)
                fprintf(stderr, "ERROR : %s, please retry later...\n", strerror(ret));
        else if (ret == ENOENT)
                printf("No license found.\n");
        else if (ret == ETIME)
                printf("License expired.\n");
        else if (ret == ENOSPC)
                printf("Excess capacity.\n");
        else if (ret == ENONET)
                printf("Node offline.\n");
        else
                printf("Invalid license.\n");

        return ret;
}

int main(int argc, char *argv[])
{
        int ret, verbose = 0;
        char c_opt, *outfile = NULL;
        enum mode_t {
                MODE_NULL,
                MODE_SNIFFER,
                MODE_LIST,
        } mode = MODE_NULL;

        dbg_info(0);

        (void) verbose;

        while (srv_running) {
                int option_index = 0;

                static struct option long_options[] = {
                        { "verbose", 0, 0, 'v' },
                        { "help",    0, 0, 'h' },
                        { "mode",    1, 0, 'm' },
                        //{ "outfile", 1, 0, 'o' },
                        { 0, 0, 0, 0 },
                };

                c_opt = getopt_long(argc, argv, "vhm:", long_options, &option_index);
                if (c_opt == -1)
                        break;

                switch (c_opt) {
                case 'v':
                        verbose = 1;
                        break;
                case 'h':
                        usage();
                        exit(0);
                case 'm':
                        if (!strcmp(optarg, "sniffer"))
                                mode = MODE_SNIFFER;
                        else if (!strcmp(optarg, "list"))
                                mode = MODE_LIST;
                        break;
                /*
                case 'o':
                        outfile = optarg;
                        break;
                */
                default:
                        usage();
                        EXIT(EINVAL);
                }
        }

        ret = env_init_simple("lich");
        if (ret)
                GOTO(err_ret, ret);

        ret = network_connect_master();
        if (ret) {
                GOTO(err_ret, ret);
        }

        switch (mode) {
        case MODE_SNIFFER:
                /*
                if (!outfile) {
                        fprintf(stderr, "outfile must be specified when sniffer\n");
                        EXIT(EINVAL);
                }
                */

                ret = __sniffer(outfile);
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case MODE_LIST:
                ret = __decrypt();
                if (ret)
                        GOTO(err_ret, ret);

                break;
        case MODE_NULL:
        default:
                fprintf(stderr, "--mode must be specified\n");
                EXIT(EINVAL);
        }

        return 0;
err_ret:
        return ret;
}
