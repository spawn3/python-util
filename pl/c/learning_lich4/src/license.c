#include "config.h"

#include <stdlib.h>
#include <openssl/md5.h>
#include <openssl/aes.h>
#include <dirent.h>

#define DBG_SUBSYS S_LIBYLIB

#include "configure.h"
#include "sysy_lib.h"
#include "sysy_conf.h"
#include "lich_md.h"
#include "storage.h"
#include "license.h"
#include "dbg.h"
#include "lich_md.h"
#include "lichstor.h"
#include "md_map.h"

#define SECONDS_PER_MONTH               (60 * 60 * 24 * 30)

/**
 * LICENSE FORMAT:
 * +-----------------------------------+
 * | 32-bytes AES encrypted time (hex) |
 * | 32-bytes MD5 encrypted MAC1 (hex) |
 * | 32-bytes MD5 encrypted MAC2 (hex) |
 * |              ...                  |
 * +-----------------------------------+
 */
#define LICENSE_MAC_OFFSET              32
#define LICENSE_CAPACITY_OFFSET         (32 * 3)
#define LICENSE_SYS_MAC_LENGTH          17      /* xx:xx:xx:xx:xx:xx */

#define LICENSE_S2HLEN(len)             ((len) * 2)
#define LICENSE_H2SLEN(len)             ((len) / 2)
#define LICENSE_HEXFMT                  "%02x"

#define LICENSE_MD5_HEXLEN              (LICENSE_S2HLEN(MD5_DIGEST_LENGTH))
#define LICENSE_AES_HEXLEN              (LICENSE_S2HLEN(AES_BLOCK_SIZE))

/* echo -n 'lich' | md5sum */
#define LICENSE_AES_KEY         "30b416e71f32b85b82694a17587d4a7b"
#define LICENSE_VERSION_2       "7780c7f41e08472e8ab4380b8a205449"
#define LICENSE_Y               ((int)'y')
#define LICENSE_F               ((int)'f')
#define LICENSE_S               ((int)'s')
#define LICENSE_ENCRYPT(time)   ((((time) + LICENSE_Y) * LICENSE_F) - LICENSE_S)
#define LICENSE_DECRYPT(time)   ((((time) + LICENSE_S) / LICENSE_F) - LICENSE_Y)

static const char *permanent_free[] = {
        "62c54c4437ba46fb94fbba05c92a3b2b",
        "ce6eedc9e7f947189690dd85a9885787",
        "41bd30ec1139499fb8f4e773505e4c43",
        "f55e6e4ed67145b79ed09ed9d50f5829",
        "6ea9c87fa3e14bf687a0c44b3b9bc7e4",
        "06941e2e0e5a4cf6b64f8b17d07c7b05",
        "f938ee57b8a54233be883c31846bc25e",
        "45e6837cdbd048ba8c8ff19df16adc50",
        "c166c37d4b47408b88fd157181360fb6",
        "f737acd64bd1484ea3e511e8049f0c71"
};

static const char *infinite_capacity[] = {
        "4116b6f52a934cd58c536dbc31bf4dbc",
        "72252f73fc5a4283a4962deae068b513",
        "c64b1943a7e548e6b7a4d5cc7f950af6",
        "acbcbd9e65414b44bbb1169cf666c43b",
        "d0566cad8ec2483a90f7fd27393434f0",
        "24967bb733c540838a8c9a4de511eb4d",
        "132b89ac3712486f8f9dedc07746da5b",
        "13dfc46980fe4e0ca6819ba6ec10514b",
        "f5f6ad3abe984e71ae817ded2376338c",
        "b4fa743d27b04e53b6e7e03be2356565"
};

static int __str2hex(const uint8_t *str, size_t slen, uint8_t *hex, size_t hlen)
{
        size_t i;

        if (LICENSE_S2HLEN(slen) <= hlen) {
                for (i = 0; i < slen; ++i) {
                        snprintf((char *)hex + i * 2, 3, LICENSE_HEXFMT, str[i]);
                }

                return 0;
        } else {
                return EINVAL;
        }
}

static int __hex2str(const uint8_t *hex, size_t hlen, uint8_t *str, size_t slen)
{
        size_t i;

        if (!(hlen % 2) && LICENSE_H2SLEN(hlen) <= slen) {
                for (i = 0; i < hlen; i += 2) {
                        sscanf((char *)hex + i, LICENSE_HEXFMT, (int *)&str[i / 2]);
                }
                str[i / 2] = 0;

                return 0;
        } else {
                return EINVAL;
        }
}

static int __arrcmp(const char **array, size_t alen, uint8_t *str, size_t slen)
{
        uint8_t i;

        for (i=0; i<alen; i++) {
                if (!memcmp(array[i], str, slen)) {
                        return 0;
                }
        }

        return -1;
}

static int __license_check_mac_one(int fd, uint8_t *_md)
{
        int ret, size, offset, cnt, valid = 0;
        struct stat stbuf;
        uint8_t md[MD5_DIGEST_LENGTH];
        uint8_t aes[AES_BLOCK_SIZE], aes_hex[LICENSE_AES_HEXLEN];

        AES_KEY key;

        ret = fstat(fd, &stbuf);
        if (ret < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        if (!S_ISREG(stbuf.st_mode)) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        size = stbuf.st_size - LICENSE_MAC_OFFSET;
        if (size % LICENSE_AES_HEXLEN) {
                ret = EINVAL;
                GOTO(err_ret, ret);
        }

        YASSERT(MD5_DIGEST_LENGTH == AES_BLOCK_SIZE);

        ret = AES_set_decrypt_key((uint8_t *)LICENSE_AES_KEY, 128, &key);
        if (ret < 0) {
                ret = EFAULT;
                GOTO(err_ret, ret);
        }

        ret = lseek(fd, 0, SEEK_SET);
        if (ret < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        cnt = read(fd, aes_hex, sizeof(aes_hex));
        if (cnt != sizeof(aes_hex)) {
                ret = EIO;
                GOTO(err_ret, ret);
        }

        if (!memcmp(aes_hex, LICENSE_VERSION_2, sizeof(aes_hex)))
                offset = LICENSE_CAPACITY_OFFSET;
        else
                offset = LICENSE_MAC_OFFSET;

        for (; offset < stbuf.st_size; offset += LICENSE_AES_HEXLEN) {
                ret = lseek(fd, offset, SEEK_SET);
                if (ret < 0) {
                        ret = errno;
                        GOTO(err_ret, ret);
                }

                cnt = read(fd, aes_hex, sizeof(aes_hex));
                if (cnt != sizeof(aes_hex))
                        continue;

                ret = __hex2str(aes_hex, sizeof(aes_hex), aes, sizeof(aes));
                if (ret)
                        GOTO(err_ret, ret);

                AES_decrypt((uint8_t *)aes, (uint8_t *)md, &key);

                if (memcmp(md, _md, sizeof(md)))
                        continue;
                else {
                        valid = 1;
                        break;
                }
        }

        return (valid ? 0 : EFAULT);
err_ret:
        return ret;
}

static int __license_check_mac(int _fd)
{
        int ret, fd, cnt, valid = 0;
        DIR *dir;
        struct dirent *ent;
        char path[MAX_PATH_LEN], parent[] = "/sys/class/net";
        uint8_t mac[LICENSE_SYS_MAC_LENGTH], md[MD5_DIGEST_LENGTH];

        dir = opendir(parent);
        if (!dir) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        while ((ent = readdir(dir))) {
                if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "lo"))
                        continue;

                snprintf(path, MAX_PATH_LEN, "%s/%s/address", parent, ent->d_name);

                fd = open(path, O_RDONLY);
                if (fd < 0)
                        continue;

                cnt = read(fd, mac, sizeof(mac));

                close(fd);

                if (cnt != sizeof(mac))
                        continue;

                MD5(mac, sizeof(mac), md);

                ret = __license_check_mac_one(_fd, md);
                if (ret)
                        continue;
                else {
                        /* One valid is ok */
                        valid = 1;
                        break;
                }
        }

        closedir(dir);

        return (valid ? 0 : EFAULT);
err_ret:
        return ret;
}

static int __license_decrypt_time(int fd, time_t *_limit)
{
        int ret, cnt;
        time_t limit;
        uint8_t epad[AES_BLOCK_SIZE];
        uint8_t etime[AES_BLOCK_SIZE], etime_hex[LICENSE_AES_HEXLEN];
        AES_KEY key;

        ret = lseek(fd, 0, SEEK_SET);
        if (ret < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        cnt = read(fd, etime_hex, sizeof(etime_hex));
        if (cnt != sizeof(etime_hex)) {
                ret = EIO;
                GOTO(err_ret, ret);
        }

        if (!memcmp(etime_hex, LICENSE_VERSION_2, sizeof(etime_hex))) {
                cnt = pread(fd, etime_hex, sizeof(etime_hex), sizeof(etime_hex) * 2);
                if (cnt != sizeof(etime_hex)) {
                        ret = EIO;
                        GOTO(err_ret, ret);
                }
        }

        if (!__arrcmp(permanent_free, 10, etime_hex, sizeof(etime_hex))) {
               *_limit = -1;
               return 0;
        }

        ret = __hex2str(etime_hex, sizeof(etime_hex), etime, sizeof(etime));
        if (ret)
                GOTO(err_ret, ret);

        ret = AES_set_decrypt_key((uint8_t *)LICENSE_AES_KEY, 128, &key);
        if (ret < 0) {
                ret = EFAULT;
                GOTO(err_ret, ret);
        }

        AES_decrypt((uint8_t *)etime, (uint8_t *)epad, &key);

        memcpy(&limit, epad, sizeof(time_t));

        *_limit = LICENSE_DECRYPT(limit);

        return 0;
err_ret:
        return ret;
}

static int __license_decrypt_capacity(int fd, long *permit_capacity)
{
        int ret, cnt;
        long _permit_capacity;
        uint8_t capacity[AES_BLOCK_SIZE] = {0};
        uint8_t ecapacity[AES_BLOCK_SIZE], ecapacity_hex[LICENSE_AES_HEXLEN];
        AES_KEY key;
        char *unit = NULL;

        ret = lseek(fd, 0, SEEK_SET);
        if (ret < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        cnt = read(fd, ecapacity_hex, sizeof(ecapacity_hex));
        if (cnt != sizeof(ecapacity_hex)) {
                ret = EIO;
                GOTO(err_ret, ret);
        }

        if (!memcmp(ecapacity_hex, LICENSE_VERSION_2, sizeof(ecapacity_hex))) {
                cnt = read(fd, ecapacity_hex, sizeof(ecapacity_hex));
                if (cnt != sizeof(ecapacity_hex)) {
                        ret = EIO;
                        GOTO(err_ret, ret);
                }
        } else {
                *permit_capacity = -1;
                return 0;
        }

        if (!__arrcmp(infinite_capacity, 10, ecapacity_hex, sizeof(ecapacity))) {
                *permit_capacity = -1;
                return 0;
        }

        ret = __hex2str(ecapacity_hex, sizeof(ecapacity_hex), ecapacity, sizeof(ecapacity));
        if (ret)
                GOTO(err_ret, ret);

        ret = AES_set_decrypt_key((uint8_t *)LICENSE_AES_KEY, 128, &key);
        if (ret < 0) {
                ret = EFAULT;
                GOTO(err_ret, ret);
        }

        AES_decrypt((uint8_t *)ecapacity, (uint8_t *)capacity, &key);

        unit = strchr((char *)capacity, 'G');
        if (unit) {
                *unit = 0;
                _permit_capacity = atoll((char *)capacity);
                _permit_capacity = _permit_capacity * (1000 * 1000 * 1000LL);
        } else {
                _permit_capacity = atoll((char *)capacity);
        }

        if (_permit_capacity < 0) {
                *permit_capacity = 0;
        } else
                *permit_capacity = _permit_capacity;

        return 0;
err_ret:
        return ret;
}

static int __license_encrypt_time(int fd, int month)
{
        int ret, cnt;
        AES_KEY key;
        time_t now, limit;
        uint8_t epad[AES_BLOCK_SIZE] = { 0 };
        uint8_t etime[AES_BLOCK_SIZE], etime_hex[LICENSE_AES_HEXLEN];

        now = time(NULL);
        limit = now + month * SECONDS_PER_MONTH;
        limit = LICENSE_ENCRYPT(limit);

        memcpy(epad, &limit, sizeof(time_t));

        ret = AES_set_encrypt_key((uint8_t *)LICENSE_AES_KEY, 128, &key);
        if (ret < 0) {
                ret = EFAULT;
                GOTO(err_ret, ret);
        }

        AES_encrypt((uint8_t *)epad, (uint8_t *)etime, &key);

        ret = __str2hex(etime, sizeof(etime), etime_hex, sizeof(etime_hex));
        if (ret)
                GOTO(err_ret, ret);

        cnt = write(fd, etime_hex, sizeof(etime_hex));
        if (cnt != sizeof(etime_hex)) {
                ret = EIO;
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __license_encrypt_time_m(char *etime_hex, int size, int month, char *time_hex)
{
        int ret, random;
        AES_KEY key;
        time_t now, limit;
        uint8_t epad[AES_BLOCK_SIZE] = { 0 }, aes_hex[LICENSE_AES_HEXLEN];
        uint8_t etime[AES_BLOCK_SIZE];
        char time_str[AES_BLOCK_SIZE + 1] = {0};

        YASSERT(size > LICENSE_AES_HEXLEN);

        memset(etime_hex, 0, size);

        if (month == 0) {
                srand((int) time(0));
                random = rand()%10;

                strcpy(etime_hex, permanent_free[random]);

                return 0;
        }

        if (time_hex) {
                YASSERT(LICENSE_AES_HEXLEN == strlen(time_hex));

                memcpy(aes_hex, time_hex, sizeof(aes_hex));

                ret = __hex2str(aes_hex, sizeof(aes_hex), epad, sizeof(epad));
                if (ret)
                        GOTO(err_ret, ret);

                ret = AES_set_decrypt_key((uint8_t *)LICENSE_AES_KEY, 128, &key);
                if (ret < 0) {
                        ret = EFAULT;
                        GOTO(err_ret, ret);
                }

                AES_decrypt((uint8_t *)epad, (uint8_t *)etime, &key);

                memcpy(time_str, etime, sizeof(etime));
                now = atoll(time_str);
        } else
                now = time(NULL);
        limit = now + month * SECONDS_PER_MONTH;
        limit = LICENSE_ENCRYPT(limit);

        memcpy(epad, &limit, sizeof(time_t));

        ret = AES_set_encrypt_key((uint8_t *)LICENSE_AES_KEY, 128, &key);
        if (ret < 0) {
                ret = EFAULT;
                GOTO(err_ret, ret);
        }

        AES_encrypt((uint8_t *)epad, (uint8_t *)etime, &key);

        ret = __str2hex(etime, sizeof(etime), (uint8_t *)etime_hex, size);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int __license_encrypt_mac(int _fd, const char *mac_file)
{
        int ret, fd, cnt;
        struct stat stbuf;
        off_t size;
        uint8_t aes[AES_BLOCK_SIZE], aes_hex[LICENSE_AES_HEXLEN];
        uint8_t md[MD5_DIGEST_LENGTH], md_hex[LICENSE_MD5_HEXLEN];
        AES_KEY key;

        fd = open(mac_file, O_RDONLY);
        if (fd < 0) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        ret = fstat(fd, &stbuf);
        if (ret < 0) {
                ret = errno;
                GOTO(err_close, ret);
        }

        size = stbuf.st_size;

        if (size % LICENSE_MD5_HEXLEN || size > MAX_BUF_LEN) {
                ret = EINVAL;
                GOTO(err_close, ret);
        }

        YASSERT(MD5_DIGEST_LENGTH == AES_BLOCK_SIZE);

        ret = AES_set_encrypt_key((uint8_t *)LICENSE_AES_KEY, 128, &key);
        if (ret < 0) {
                ret = EFAULT;
                GOTO(err_close, ret);
        }

        while (size) {
                cnt = read(fd, md_hex, sizeof(md_hex));
                if (cnt != sizeof(md_hex)) {
                        ret = EIO;
                        GOTO(err_close, ret);
                }

                ret = __hex2str(md_hex, sizeof(md_hex), md, sizeof(md));
                if (ret)
                        GOTO(err_ret, ret);

                AES_encrypt((uint8_t *)md, (uint8_t *)aes, &key);

                ret = __str2hex(aes, sizeof(aes), aes_hex, sizeof(aes_hex));
                if (ret)
                        GOTO(err_ret, ret);

                cnt = write(_fd, aes_hex, sizeof(aes_hex));
                if (cnt != sizeof(aes_hex)) {
                        ret = EIO;
                        GOTO(err_close, ret);
                }

                size -= cnt;
        }

        close(fd);

        return 0;
err_close:
        close(fd);
err_ret:
        return ret;
}

static int __license_encrypt_mac_m(const char *md_index, FILE *fp_lic)
{
        int ret, size, cnt;
        uint8_t aes[AES_BLOCK_SIZE], aes_hex[LICENSE_AES_HEXLEN];
        uint8_t md[MD5_DIGEST_LENGTH], md_hex[LICENSE_MD5_HEXLEN];
        AES_KEY key;

        YASSERT(MD5_DIGEST_LENGTH == AES_BLOCK_SIZE);

        ret = AES_set_encrypt_key((uint8_t *)LICENSE_AES_KEY, 128, &key);
        if (ret < 0) {
                ret = EFAULT;
                GOTO(err_ret, ret);
        }

        size = strlen(md_index);
        while (size > 0) {
                if (size < LICENSE_MD5_HEXLEN) {
                        ret = EIO;
                        GOTO(err_ret, ret);
                }

                memcpy(md_hex, md_index, sizeof(md_hex));

                ret = __hex2str(md_hex, sizeof(md_hex), md, sizeof(md));
                if (ret)
                        GOTO(err_ret, ret);

                AES_encrypt((uint8_t *)md, (uint8_t *)aes, &key);

                ret = __str2hex(aes, sizeof(aes), aes_hex, sizeof(aes_hex));
                if (ret)
                        GOTO(err_ret, ret);

                cnt = fwrite(aes_hex, 1, sizeof(aes_hex), fp_lic);
                if (cnt != sizeof(aes_hex)) {
                        ret = EIO;
                        GOTO(err_ret, ret);
                }

                size -= cnt;
                md_index += cnt;
        }

        return 0;
err_ret:
        return ret;
}

static int __license_encrypt_capacity(uint64_t disk_size, char *edisk_size, size_t len)
{
        int ret;
        uint8_t aes[AES_BLOCK_SIZE], aes_hex[LICENSE_AES_HEXLEN];
        uint8_t capacity[AES_BLOCK_SIZE];
        AES_KEY key;

        ret = AES_set_encrypt_key((uint8_t *)LICENSE_AES_KEY, 128, &key);
        if (ret < 0) {
                ret = EFAULT;
                GOTO(err_ret, ret);
        }

        memset(capacity, 0, sizeof(capacity));
        snprintf((char *)capacity, sizeof(capacity), "%ld", disk_size);

        AES_encrypt((uint8_t *)capacity, (uint8_t *)aes, &key);

        ret = __str2hex(aes, sizeof(aes), aes_hex, sizeof(aes_hex));
        if (ret)
                GOTO(err_ret, ret);

        YASSERT(sizeof(aes_hex) < len);

        memcpy(edisk_size, aes_hex, sizeof(aes_hex));
        edisk_size[sizeof(aes_hex)] = '\0';

        return 0;
err_ret:
        return ret;
}

static int __license_sniffer_time(char *etime, size_t len)
{
        int ret;
        time_t now;
        uint8_t aes[AES_BLOCK_SIZE], aes_hex[LICENSE_AES_HEXLEN];
        uint8_t capacity[AES_BLOCK_SIZE];
        AES_KEY key;

        ret = AES_set_encrypt_key((uint8_t *)LICENSE_AES_KEY, 128, &key);
        if (ret < 0) {
                ret = EFAULT;
                GOTO(err_ret, ret);
        }

        now = time(NULL);
        memset(capacity, 0, sizeof(capacity));
        snprintf((char *)capacity, sizeof(capacity), "%ld", now);

        AES_encrypt((uint8_t *)capacity, (uint8_t *)aes, &key);

        ret = __str2hex(aes, sizeof(aes), aes_hex, sizeof(aes_hex));
        if (ret)
                GOTO(err_ret, ret);

        YASSERT(sizeof(aes_hex) < len);

        memcpy(etime, aes_hex, sizeof(aes_hex));
        etime[sizeof(aes_hex)] = '\0';

        return 0;
err_ret:
        return ret;
}

static int __license_encrypt_capacity_m(const char *capacity_index, int _capacity, FILE *fp_lic)
{
        int ret, random, cnt;
        char capacity_str[AES_BLOCK_SIZE + 1] = {0};
        uint64_t disk_size;
        uint8_t aes[AES_BLOCK_SIZE], aes_hex[LICENSE_AES_HEXLEN];
        uint8_t capacity[AES_BLOCK_SIZE];
        AES_KEY key;

        if (_capacity == 0) {
                srand((int) time(0));
                random = rand()%10;

                cnt = fwrite(infinite_capacity[random], 1, 32, fp_lic);
                if (cnt != 32) {
                        ret = EIO;
                        GOTO(err_ret, ret);
                }

                return 0;
        }

        YASSERT(LICENSE_AES_HEXLEN == strlen(capacity_index));

        memcpy(aes_hex, capacity_index, sizeof(aes_hex));

        ret = __hex2str(aes_hex, sizeof(aes_hex), aes, sizeof(aes));
        if (ret)
                GOTO(err_ret, ret);

        ret = AES_set_decrypt_key((uint8_t *)LICENSE_AES_KEY, 128, &key);
        if (ret < 0) {
                ret = EFAULT;
                GOTO(err_ret, ret);
        }

        AES_decrypt((uint8_t *)aes, (uint8_t *)capacity, &key);

        memcpy(capacity_str, capacity, sizeof(aes));

        if (_capacity == -1) {
                disk_size = atoll(capacity_str);
                disk_size += 200;/* 200G */
        } else if (_capacity != 0) {
                disk_size = _capacity;
        }

        snprintf((char *)capacity, sizeof(capacity), "%ldG", disk_size);

        ret = AES_set_encrypt_key((uint8_t *)LICENSE_AES_KEY, 128, &key);
        if (ret < 0) {
                ret = EFAULT;
                GOTO(err_ret, ret);
        }

        AES_encrypt((uint8_t *)capacity, (uint8_t *)aes, &key);

        ret = __str2hex(aes, sizeof(aes), aes_hex, sizeof(aes_hex));
        if (ret)
                GOTO(err_ret, ret);

        cnt = fwrite(aes_hex, 1, sizeof(aes_hex), fp_lic);
        if (cnt != sizeof(aes_hex)) {
                ret = EIO;
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __license_sniffer_capacity(long *size)
{
        int ret;
        DIR *dir;
        struct dirent *ent;
        char path[MAX_PATH_LEN], parent[MAX_PATH_LEN];
        uint64_t total;

        long tmp = 0;
        *size = 0;

        ret = conf_init();
        if (ret)
                GOTO(err_ret, ret);

        dbg_sub_init();        

        strcpy(parent, gloconf.home);
        strcat(parent, "/data/disk/disk");

        dir = opendir(parent);
        if (!dir) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        while ((ent = readdir(dir))) {
                if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
                        continue;

                snprintf(path, sizeof(path), "%s/%s", parent, ent->d_name);

                ret = _disk_dfree_link(path, &total);
                if (ret)
                        GOTO(err_close, ret);

                tmp = total - cdsconf.disk_keep;
                *size += (tmp > 0 ? tmp : 0);
        }

        closedir(dir);

        return 0;
err_close:
        closedir(dir);
err_ret:
        return ret;
}

static int __license_sniffer_mac(char *mac_hex, int len)
{
        int ret, fd, cnt, valid = 0;
        DIR *dir;
        struct dirent *ent;
        char path[MAX_PATH_LEN], parent[] = "/sys/class/net";
        uint8_t mac[LICENSE_SYS_MAC_LENGTH];
        uint8_t md[MD5_DIGEST_LENGTH], md_hex[LICENSE_MD5_HEXLEN];

        memset(mac_hex, 0, len);

        dir = opendir(parent);
        if (!dir) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        while ((ent = readdir(dir))) {
                if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "lo"))
                        continue;

                snprintf(path, sizeof(path), "%s/%s/address", parent, ent->d_name);

                fd = open(path, O_RDONLY);
                if (fd < 0)
                        continue;

                cnt = read(fd, mac, sizeof(mac));
                if (cnt != sizeof(mac))
                        continue;

                close(fd);

                MD5(mac, sizeof(mac), md);

                ret = __str2hex(md, sizeof(md), md_hex, sizeof(md_hex));
                if (ret)
                        GOTO(err_close, ret);

                YASSERT(((valid + 1) * (int)sizeof(md_hex)) < len);

                memcpy(mac_hex + valid * sizeof(md_hex), md_hex, sizeof(md_hex));

                /*
                cnt = write(_fd, md_hex, sizeof(md_hex));
                if (cnt != sizeof(md_hex)) {
                        ret = EIO;
                        GOTO(err_close, ret);
                }
                */

                ++valid;
        }

        if (!valid) {
                ret = ENOENT;
                GOTO(err_close, ret);
        }

        closedir(dir);

        return 0;
err_close:
        closedir(dir);
err_ret:
        return ret;
}

/*
 * license_decrypt -- Decrypt license time
 */
int license_decrypt(int fd, time_t *time)
{
        return __license_decrypt_time(fd, time);
}

/*
 * license_encrypt -- Create licese file
 *
 * @fd: license file descriptor
 * @mac_file: file contained md5 of mac
 * @month: license time
 */
int license_encrypt(int fd, const char *mac_file, int month)
{
        int ret;

        ret = __license_encrypt_time(fd, month);
        if (ret)
                GOTO(err_ret, ret);

        ret = __license_encrypt_mac(fd, mac_file);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

/*
 * license_encrypt_m -- Mass create licese file
 *
 * @mac_file: file contained md5 of mac, format like: hostname XXXXXXXXXXXXXXXXX...
 * @month: license time
 */
int license_encrypt_m(const char *mac_file, const char *odir, int month, int capacity)
{
        int ret;
        FILE *fp_mac, *fp_lic;
        struct stat stbuf;
        char buf[MAX_LINE_LEN], outfile[MAX_NAME_LEN], *hostname, *md_index, *capacity_index;
        char etime_hex[LICENSE_AES_HEXLEN+1], *time_index;

        fp_mac = fopen(mac_file, "r");
        if (!fp_mac) {
                ret = errno;
                GOTO(err_ret, ret);
        }

        while (fgets(buf, MAX_LINE_LEN, fp_mac)) {
                if (buf[strlen(buf)-1] == '\n')
                        buf[strlen(buf)-1] = '\0';

                hostname = buf;
                md_index = strchr(hostname, ' ');
                if (!md_index) {
                        ret = EIO;
                        GOTO(err_close, ret);
                }

                *md_index = '\0';
                md_index++;

                if (!strncmp(hostname, "lich_version:", strlen("lich_version:"))) {
                        hostname = md_index;
                        md_index = strchr(hostname, ' ');
                        if (!md_index) {
                                ret = EIO;
                                GOTO(err_close, ret);
                        }

                        *md_index = '\0';
                        md_index++;
                }

                memset(outfile, 0, sizeof(outfile));
                if (odir) {
                        strcpy(outfile, odir);

                        ret = stat(outfile, &stbuf);
                        if (ret < 0) {
                                ret = errno;
                                GOTO(err_close, ret);
                        }

                        if(!S_ISDIR(stbuf.st_mode)) {
                                ret = ENOTDIR;
                                GOTO(err_close, ret);
                        }

                        if (outfile[strlen(outfile)-1] != '/')
                                strcat(outfile, "/");
                }

                strcat(outfile, hostname);

                fp_lic = fopen(outfile, "w");
                if (!fp_lic) {
                        ret = errno;
                        GOTO(err_close, ret);
                }

                capacity_index = strchr(md_index, ' ');
                if (!capacity_index) {
                        ret = __license_encrypt_time_m(etime_hex, sizeof(etime_hex), month, NULL);
                        if (ret)
                                GOTO(err_ret, ret);

                        fputs(etime_hex, fp_lic);
                } else {
                        *capacity_index = '\0';
                        capacity_index++;

                        time_index = strchr(capacity_index, ' ');
                        if (time_index) {
                                *time_index = '\0';
                                time_index++;
                        }

                        fputs(LICENSE_VERSION_2, fp_lic);

                        ret = __license_encrypt_capacity_m(capacity_index, capacity, fp_lic);
                        if (ret)
                                GOTO(err_close_lic, ret);

                        ret = __license_encrypt_time_m(etime_hex, sizeof(etime_hex), month, time_index);
                        if (ret)
                                GOTO(err_ret, ret);

                        fputs(etime_hex, fp_lic);
                }

                ret = __license_encrypt_mac_m(md_index, fp_lic);
                if (ret)
                        GOTO(err_close_lic, ret);

                fclose(fp_lic);
        }

        fclose(fp_mac);

        return 0;
err_close_lic:
        fclose(fp_lic);
err_close:
        fclose(fp_mac);
err_ret:
        return ret;
}

/*
 * license_sniffer -- store system infomation to terminal --file
 *
 * @_fd: file to store md5 of mac
 * output format: hostname FFFFFFFFFFFFFFFFFFFFFFF(machine code hex) FFFFFFFFFF(disk size hex) FFFFFFFF(machine time hex)
 */
int license_sniffer(int _fd)
{
        int ret;
        long disk_size;
        char edisk_size[MAX_BUF_LEN], etime[MAX_BUF_LEN];
        char hostname[MAX_NAME_LEN+1], mac_hex[MAX_BUF_LEN];
        (void) _fd;

        net_gethostname(hostname, MAX_NAME_LEN);
        printf("%s ", hostname);

        ret = __license_sniffer_mac(mac_hex, MAX_BUF_LEN);
        if (ret)
                GOTO(err_ret, ret);
        printf("%s ", mac_hex);

        ret = __license_sniffer_capacity(&disk_size);
        if (ret)
                GOTO(err_ret, ret);

        ret = __license_encrypt_capacity(disk_size, edisk_size, sizeof(edisk_size));
        if (ret)
                GOTO(err_ret, ret);
        printf("%s ", edisk_size);

        ret = __license_sniffer_time(etime, sizeof(etime));
        if (ret)
                GOTO(err_ret, ret);
        printf("%s\n", etime);

        return 0;
err_ret:
        return ret;
}

/*
 * license_check -- Check license validity
 */
int license_check(const char *home, long *permit_capacity, int free_limit)
{
        int ret, fd;
        time_t now, limit;
        long disk_size;
        char path[MAX_PATH_LEN];

        snprintf(path, MAX_PATH_LEN, "%s/license", home);
        fd = open(path, O_RDONLY);
        if (fd < 0) {
                //DERROR("no license found\n");
                ret = ENOENT;
                GOTO(err_ret, ret);
        }

        ret = __license_decrypt_time(fd, &limit);
        if (ret) {
                //DERROR("wrong license format\n");
                GOTO(err_close, ret);
        }

        now = time(NULL);
        if (limit != -1 && now > limit) {
                //DERROR("check license validity failed\n");
                ret = ETIME;
                GOTO(err_close, ret);
        }

        ret = __license_decrypt_capacity(fd, permit_capacity);
        if (ret)
                GOTO(err_close, ret);

        if (*permit_capacity != -1) {
                ret = __license_sniffer_capacity(&disk_size);
                if (ret)
                        GOTO(err_close, ret);
        }

        if (!free_limit && *permit_capacity != -1 && disk_size > *permit_capacity) {
                ret = ENOSPC;
                GOTO(err_close, ret);
        }

        ret = __license_check_mac(fd);
        if (ret) {
                //DERROR("check license hardware failed\n");
                GOTO(err_close, ret);
        }

        close(fd);

        return 0;
err_close:
        close(fd);
err_ret:
        //exit(EACCES);
        return ret;
}

int storage_get_createtime(uint32_t *createtime)
{
        int ret, buflen, retry = 0;
        char buf[MAX_BUF_LEN];
        fileid_t fileid;
        nid_t nid;

        ret = stor_init(NULL, -1);
        if (ret)
                GOTO(err_ret, ret);

retry:
        ret = stor_lookup1(SYSTEM_ROOT, &fileid);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else if (ret == ENOKEY || ret == ENOENT) {
                        *createtime = time(NULL);
                        goto out;
                } else {
                        GOTO(err_ret, ret);
                }
        }

        buf[0] = '\0';
        buflen = MAX_BUF_LEN;

        ret = md_map_getsrv(&fileid, &nid);
        if (ret)
                GOTO(err_ret, ret);

        ret = md_xattr_get(&nid, &fileid, LICH_SYSTEM_ATTR_CREATETIME, buf, &buflen);
        if (ret) {
                if (ret == ENOKEY || ret == ENOENT) {
                        *createtime = time(NULL);
                        goto out;
                } else
                        GOTO(err_ret, ret);
        }

        *createtime = atol(buf);

out:
        return 0;
err_ret:
        return ret;
}

int storage_license_check()
{
        int ret, now, retry = 0, free_limit = 0;
        long capacity;
        uint32_t createtime;
	struct stat buf;
        char license[MAX_PATH_LEN];

#if 0
        return 0;
#endif

	if (cluster_is_solomode())
		return 0;

retry:
        ret = storage_get_createtime(&createtime);
        if (ret) {
                if (ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        now = time(NULL);
        snprintf(license, MAX_PATH_LEN, "%s/license", gloconf.home);
        ret = stat(license, &buf);
        if (!ret) {
                if ((buf.st_size == 0) && (now - createtime <= FREE_LICENSE)) {
                        return 0;
                }
        }

        ret = license_check(gloconf.home, &capacity, free_limit);
        if (ret) {
                if (ret == ENOENT) {
                        if (now - createtime > FREE_LICENSE) {
                                DERROR("No license found.\n");
                        }
                        else
                                return 0;
                } else if (ret == ETIME) {
                        DERROR("License expired.\n");
                } else if (ret == ENOSPC) {
                        DERROR("Excess capacity.\n");
                } else {
                        DERROR("Invalid license.\n");
                }

                exit(EACCES);
        }

        return 0;
err_ret:
        return ret;
}
