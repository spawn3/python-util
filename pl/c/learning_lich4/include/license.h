#ifndef LICENSE_H
#define LICENSE_H

#define FREE_LICENSE (60 * 60 * 24 * 30 * 3)

extern int license_check(const char *home, long *capacity, int free_limit);
extern int storage_license_check(void);
extern int storage_get_createtime(uint32_t *createtime);
extern int license_sniffer(int fd);
extern int license_encrypt(int fd, const char *mac_file, int month);
extern int license_encrypt_m(const char *mac_file, const char *odir, int month, int capacity);
extern int license_decrypt(int fd, time_t *time);

#endif /* LICENSE_H */
