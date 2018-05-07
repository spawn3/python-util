#ifndef __UTILS_H_
#define __UTILS_H_

#include "chunk.h"

typedef struct {
        struct list_head hook;
        nid_t master;
        uint64_t taskid;
        chkinfo_t chkinfo[0];
} scan_entry_t;

static inline int check_nodes(char nodes[][1024], int count)
{
        int i, j;

        if (count == 1)
                return 0;

        for (i = 0; i < count; i++) {
                for (j = i + 1; j < count; j++) {
                        if(strcmp(nodes[i] , nodes[j]) == 0)  {
                            return 1;
                        };
                }
        }

        return 0;
}

/* clone.c */
int clone_local2lun(const char *from, const char *to, int replace);
int clone_lun2lun1(const char *from, const char *to);
int clone_lun2lun(const char *from, const char *to);
int clone_lun2local(const char *from, const char *to);
int append_local2lun(const char *from, const char *to);
int clone_init(int thread);

/* recover.c */
int recover_parallel_get(int *_parallel, int *_expdata, int *_parallel_real);
int recover_queue(msgqueue_t *queue, int total, int parallel, int *needretry, int *recover_succ);
int recover_pop(msgqueue_t *queue, int count, struct list_head *list);
int recover_replica_queue(msgqueue_t *queue, int total, int parallels, int *needretry);
int recover_chkinfo_losted(const chkinfo_t *chkinfo);

/* balance.c */
int parallel_get(int *_parallel, char *parallel_path, int *_expdata,
                char *expdata_path, int *_parallel_real, int parallel_default);
int balance_parallel_get(int *parallel, int *expdata, int *parallel_real);
int balance(const chkid_t *id);

/*  not use */
int chunk_check_node(const chkinfo_t *chkinfo, int full);
int iterate_all(const char *path, int verbose, msgqueue_t *recover,
                uint64_t *recover_count, msgqueue_t *lost, uint64_t *lost_count, int full);

int lich_chunktable_scanmeta(const char **path, int count, int idx, int recover,
                int *recover_total, int *recover_need, int *recover_succ);
int lich_chunktable_recover_replica(const char **path, int count, int idx);
int lich_meta_recover_replica(chkinfo_t *chkinfo);

/* scan.c */
int lich_scan(const char *path, int verbose, int recover, int full);

/* recovery.c */
int recovery_bypath(const char *path, int deep);
int recovery_scan(const char *path);


/* attr.c */
int lich_xattr_get(const char *path, const char *key);
int lich_xattr_remove(const char *path, const char *key);
int lich_xattr_set(const char *path, const char *key, const char *value);

/* copy.c */
int stor_copy(const char *from, const char *to, int multithreading);

int file_md5sum(const fileid_t *fileid);

/* hierarch.c */
int lich_tier_set(const char *name, int tier, int verbose);
int lich_tier_get(const char *name, int verbose);
int lich_priority_set(const char *name, int tier, int verbose, int pithy);
int lich_priority_get(const char *name, int verbose, int pithy);
int lich_localize_set(const char *name, int num);
int lich_localize_get(const char *name);
int lich_multpath_set(const char *name, int num);
int lich_multpath_get(const char *name);


int utils_mkpool(const char *dir);
int utils_rmpool(const char *dir);
int utils_list(const char *dir, int flag);

int utils_touch(const char *file);
int utils_rmvol(const char *file);
int utils_get_size(const char *_str, uint64_t *_size);
int utils_truncate(const char *path, size_t size);
int utils_list(const char *dir, int flag);
int utils_stat(const char *path, int format);
int utils_rename(const char *from, const char *to);
int utils_find(const char *path, const char *name);
int utils_cat(const char *path);
int utils_write(const char *from, const char *to);
int utils_fsstat(const char *path);

int utils_snapshot(const char *name);
int utils_snapshot_remove(const char *name);
int utils_snapshot_rollback(const char *name);
int utils_snapshot_protect(const char *name);
int utils_snapshot_unprotect(const char *name);
int utils_snapshot_list(const char *name, int all, int format);
int utils_snapshot_clone(const char *name, const char *to);
int utils_snapshot_copy(const char *name, const char *to);
int utils_snapshot_cat(const char *name);
#endif /* __UTILS_H_ */
