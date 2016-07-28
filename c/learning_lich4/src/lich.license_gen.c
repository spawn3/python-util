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

#include "configure.h"
#include "sysy_lib.h"
#include "dbg.h"
#include "license.h"

/**
 * echo -n '1@3$5^7*qWeRtYuI' | md5sum
 */
#define LICENSE_SHADOW          "6d3cf879a478a65506118fd25227bcd8"

#define LICENSE_MONTH_MIN       (0)
#define LICENSE_MONTH_MAX       (12 * 10)       /* 10 years */

static void usage()
{
        fprintf(stderr, "\nusage: lich.license_gen [-h] [-i infile -o outfile] [-m -i infile [-o outdir]]\n"
                "\t-h           Show this help\n"
                "\t-m           Mass production license. if mass, infile must be specified\n"
                "\t                         outfile is license out directroy, default is current directory\n"
                "\t-i           Machine infomation file\n"
                "\t-o           License file path\n"
                "\n"
               );
}

/**
 * Disable line buffer and input echo of stdin
 */
static int __getch()
{
        char ch;
        struct termios old, new;

        (void) tcgetattr(STDIN_FILENO, &old);

        memcpy(&new, &old, sizeof(struct termios));
        new.c_lflag &= ~(ICANON | ECHO);

        (void) tcsetattr(STDIN_FILENO, TCSANOW, &new);

        ch = getchar();

        (void) tcsetattr(STDIN_FILENO, TCSANOW, &old);

        return ch;
}

static int __enter_pass(char *pass, int len)
{
        int i, j;
        char ch;

        printf("Password: ");

        for (i = 0, j = 0; i < len - 1; ++i) {
                ch = __getch();

                if (ch == '\n' || ch == '\r')
                        break;

                if (ch == 0x7f) { /* Backspace */
                        --i;
                        if (j)
                                --j;
                } else {
                        pass[j++] = ch;
                }
        }
        pass[j] = 0;

        printf("\n");

        return 0;
}

static int __enter_month(int *_month)
{
        int ret, month = 0, i = 0;
        char buf[MAX_BUF_LEN];

        printf("Validity of License (in months): ");

        if (!fgets(buf, MAX_BUF_LEN, stdin)) {
                month = -1;
        } else {
                while(buf[i]) {
                        if (i != 0 && buf[i] == '\n') {
                        } else if (buf[i] < '0' || buf[i] > '9') {
                                fprintf(stderr, "Invalid month, valid region of month is [%d~%d], 0 means permanent\n",
                                        LICENSE_MONTH_MIN, LICENSE_MONTH_MAX);
                                ret = EINVAL;
                                GOTO(err_ret, ret);
                        }
                        i++;
                }
                month = atoll(buf);
        }

        if (month < LICENSE_MONTH_MIN || month > LICENSE_MONTH_MAX) {
                fprintf(stderr, "Invalid month, valid region of month is [%d~%d], 0 means permanent\n",
                        LICENSE_MONTH_MIN, LICENSE_MONTH_MAX);
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        *_month = month;

        return 0;
err_ret:
        return ret;
}

static int __enter_capacity(int *_capacity)
{
        int ret, capacity = 0, i = 0;
        char buf[MAX_BUF_LEN];

        printf("Validity of License (in Gigas, input enter to use info file capacity): ");

        if (!fgets(buf, MAX_BUF_LEN, stdin)) {
                capacity = -1;
        } else {
                while(buf[i]) {
                        if (buf[i] == '\n') {
                        } else if (buf[i] < '0' || buf[i] > '9') {
                                fprintf(stderr, "Invalid capacity, capacity must be a unsigned integer, 0 means infinite\n");
                                ret = EINVAL;
                                GOTO(err_ret, ret);
                        }
                        i++;
                }
                if (buf[0] == '\n')
                        capacity = -1;
                else
                        capacity = atoll(buf);
        }

        *_capacity = capacity;

        return 0;
err_ret:
        return ret;
}

static int __login()
{
        int ret, i;
        unsigned char md[MD5_DIGEST_LENGTH];
        char pass[MAX_BUF_LEN];
        char shadow[MD5_DIGEST_LENGTH * 2] = { 0 };

        ret = __enter_pass(pass, MAX_BUF_LEN);
        if (ret)
                GOTO(err_ret, ret);

        MD5((unsigned char *)pass, strlen(pass), md);

        for (i = 0; i < MD5_DIGEST_LENGTH; ++i) {
                snprintf(shadow + strlen(shadow), 3, "%02x", md[i]);
        }

        if (memcmp(LICENSE_SHADOW, shadow, MD5_DIGEST_LENGTH * 2)) {
                fprintf(stderr, "Authentication failure\n");
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __decrypt(const char *license)
{
        int ret, fd;
        time_t limit;

        fd = open(license, O_RDONLY);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        ret = license_decrypt(fd, &limit);
        if (ret)
                GOTO(err_ret, ret);

        close(fd);

        printf("License: %s", ctime(&limit));

        return 0;
err_ret:
        return ret;
}

static int __encrypt(const char *ifile, const char *ofile)
{
        int ret, fd, month;

        ret = __login();
        if (ret)
                GOTO(err_ret, ret);

        ret = __enter_month(&month);
        if (ret)
                GOTO(err_ret, ret);

        fd = open(ofile, O_CREAT | O_TRUNC | O_WRONLY, 0200);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        ret = license_encrypt(fd, ifile, month);
        if (ret)
                GOTO(err_close, ret);

        close(fd);

        __decrypt(ofile);

        return 0;
err_close:
        close(fd);
err_ret:
        return ret;
}

static int __encrypt_m(const char *ifile, const char *odir)
{
        int ret, month, capacity;

        ret = __login();
        if (ret)
                GOTO(err_ret, ret);

        ret = __enter_month(&month);
        if (ret)
                GOTO(err_ret, ret);

        ret = __enter_capacity(&capacity);
        if (ret)
                GOTO(err_ret, ret);

        ret = license_encrypt_m(ifile, odir, month, capacity);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

int main(int argc, char *argv[])
{
        int ret, verbose = 0, mass = 0;
        char c_opt, *ifile = NULL, *ofile = NULL;

        (void) verbose;

        while (srv_running) {
                int option_index = 0;

                static struct option long_options[] = {
                        { "verbose", 0, 0, 'v' },
                        { "help",    0, 0, 'h' },
                        { "mass",    0, 0, 'm' },
                        { "infile",  1, 0, 'i' },
                        { "outfile", 1, 0, 'o' },
                        { 0, 0, 0, 0 },
                };

                c_opt = getopt_long(argc, argv, "vhmi:o:", long_options, &option_index);
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
                        mass = 1;
                        break;
                case 'i':
                        ifile = optarg;
                        break;
                case 'o':
                        ofile = optarg;
                        break;
                default:
                        usage();
                        EXIT(EINVAL);
                }
        }

        if (mass == 0) {
                printf("infile:%s, ofile:%s\n", ifile, ofile);
                if (!ifile || !ofile) {
                        usage();
                        EXIT(EINVAL);
                }

                ret = __encrypt(ifile, ofile);
                if (ret)
                        GOTO(err_ret, ret);
        } else {
                if (!ifile) {
                        usage();
                        EXIT(EINVAL);
                }

                ret = __encrypt_m(ifile, ofile);
                if (ret)
                        GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}
