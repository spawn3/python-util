#include "config.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <dirent.h>
#include <ctype.h>

#include "configure.h"
#include "env.h"
#include "cluster.h"
#include "storage.h"
#include "net_global.h"
#include "net_vip.h"
#include "sysutil.h"
#include "../../storage/replica/disk.h"

typedef enum {
        OP_NULL,
        OP_SYSSTAT,
        OP_CREATE,
        OP_LISTNODE,
        OP_STAT,
        OP_SETMETA,
        OP_DROPMETA,
        OP_ADDNODE,
        OP_DROPNODE,
        OP_CASTOFF,
        OP_SETVIP,
        OP_MOVEMETA,
        OP_SETARBITOR,
        OP_CONFIGDUMP,
        OP_HOSTSDUMP,
} admin_op_t;

static int verbose = 0;
static int __load__ = 0;

static void usage()
{
        fprintf(stderr, "\nusage:\n"
                "lich.admin --sysstat\n"
                "lich.admin --configdump\n"
                "lich.admin --hostsdump\n"
                "lich.admin --create node1\n"
                "lich.admin --stat nodeX\n"
                "lich.admin --setmeta nodeX\n"
                "lich.admin --dropmeta nodeX\n"
                "lich.admin --listnode [-v, --verbose] [--load]\n"
                "lich.admin --addnode node3 node4 ... nodeX\n"
                "lich.admin --dropnode node3 node4 ... nodeX\n"
                "lich.admin --castoff <nodeX | nodeX/0>\n"
                "lich.admin --setvip\n"
               );
}

static int __admin_stat(const char *name)
{
        int ret;
        char buf[MAX_BUF_LEN];
        nodeinfo_t *info;

        YASSERT(name);

        info = (void *)buf;
        ret = node_getinfo(info, name);
        if (ret) {
                GOTO(err_ret, ret);
        }

        if ((LLU)info->stat->total <= (LLU)cdsconf.disk_keep 
                || ((LLU)info->stat->used + (LLU)cdsconf.disk_keep) >= (LLU)info->stat->total) {
                info->stat->used = info->stat->total;
        } else {
                info->stat->used += (uint64_t)cdsconf.disk_keep;
        }

        if (info->stat->status & __NODE_STAT_DELETING__) {
                printf("cluster:%s\n"
                         "hostname:%s\n"
                         "nid:"NID_FORMAT"\n"
                         "writeable:%s\n"
                         "ready:%s\n"
                         "deleting:%s\n"
                         "capacity:%llu\n"
                         "used:%llu\n"
                         "load:%llu\n"
                         "admin:%s\n"
                         "status:%s\n"
                         "home:%s\n"
                         "metas:%s\n"
                         "uptime:%d\n",
                         info->clustername,
                         info->nodename,
                         NID_ARG(&info->stat->nid),
                         info->stat->status & __NODE_STAT_WRITEABLE__ ? "True" : "False",
                         info->stat->status & __NODE_STAT_READY__ ? "True" : "False",
                         info->stat->status & __NODE_STAT_DELETING__? "True" : "False",
                         (LLU)info->stat->total,
                         (LLU)info->stat->used,
                         (LLU)info->stat->load,
                         info->admin,
                         info->status,
                         info->home,
                         info->quorum,
                         *info->uptime);
        } else {
                printf("cluster:%s\n"
                         "hostname:%s\n"
                         "nid:"NID_FORMAT"\n"
                         "writeable:%s\n"
                         "ready:%s\n"
                         "capacity:%llu\n"
                         "used:%llu\n"
                         "load:%llu\n"
                         "admin:%s\n"
                         "status:%s\n"
                         "home:%s\n"
                         "metas:%s\n"
                         "uptime:%d\n",
                         info->clustername,
                         info->nodename,
                         NID_ARG(&info->stat->nid),
                         info->stat->status & __NODE_STAT_WRITEABLE__ ? "True" : "False",
                         info->stat->status & __NODE_STAT_READY__ ? "True" : "False",
                         (LLU)info->stat->total,
                         (LLU)info->stat->used,
                         (LLU)info->stat->load,
                         info->admin,
                         info->status,
                         info->home,
                         info->quorum,
                         *info->uptime);
        }

        return 0;
err_ret:
        return ret;
}

static int __is_meta(const char *name)
{
        int ret;
        char buf[MAX_BUF_LEN];
        nodeinfo_t *info;

        YASSERT(name);

        info = (void *)buf;
        ret = node_getinfo(info, name);
        if (ret) {
                return 0;
        }

        if (strcmp(info->status, "meta") == 0) {
                return 1;
        }

        return 0;
}

static int __admin_listnode()
{
        int ret, buflen;
        char buf[MAX_BUF_LEN], tmp[MAX_BUF_LEN];
        uint64_t offset;
        struct dirent *de;
        nodeinfo_t *info;

        ret = cluster_listnode(buf, &buflen);
        if (ret) {
                GOTO(err_ret, ret);
        }

        offset = 0;
        info = (void *)tmp;
        dir_for_each(buf, buflen, de, offset) {
                ret = node_getinfo(info, de->d_name);
                if (ret) {
#if 0
                        if (verbose) {
                                printf("\x1b[1;31m%s:stopped\x1b[0m\n", de->d_name);
                        } else
                                printf("\x1b[1;31m%s\x1b[0m\n", de->d_name);
#endif
                        if (verbose) {
                                printf("%s:stopped\n", de->d_name);
                        } else
                                printf("%s\n", de->d_name);
                } else {
                        if (verbose || __load__) {
                                if (verbose) {
                                        if (info->stat->status & __NODE_STAT_DELETING__)
                                                printf("%s:%s,%s\n", de->d_name, info->status, "deleting");
                                        else
                                                printf("%s:%s\n", de->d_name, info->status);
                                } else {
                                        printf("%s:%llu\n", de->d_name, (LLU)info->stat->load);
                                }
                        } else
                                printf("%s\n", de->d_name);
                }
        }

        return 0;
err_ret:
        return ret;
}

static int __admin_dropnode(const char **list, int count)
{
        int ret, retry = 0;

retry:
        ret = network_connect_master();
        if (ret) {
                if (ret == ENONET || ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = cluster_dropnode(list, count);
        if (ret)
                GOTO(err_ret, ret);

        return 0;
err_ret:
        return ret;
}

static int __admin_castoff(const char *name)
{
        int ret, disk;
        char *c;
        char host[MAX_NAME_LEN];

        c = strchr(name, '/');
        if (c) {
                *c = '\0';
                disk = atoi(c+1);
        } else {
                disk = DISK_ALL_IDX;
        }

        ret = net_gethostname(host, MAX_MSG_SIZE);
        if (ret) {
                GOTO(err_ret, ret);
        }

        if (strcmp(name, host) != 0) {
                DERROR("the local host is: %s, need exec on host: %s\n", host, name);
                ret = 1;
                GOTO(err_ret, ret);
        }

        return node_castoff(name, disk);
err_ret:
        return ret;
}

static int __admin_setvip()
{
        int ret;

        ret = netvip_setvip();
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}

static int __admin_sysstat(int force)
{
        int ret, retry = 0;
        lich_stat_t stat;

retry:
        ret = network_connect_master();
        if (ret) {
                if (ret == ENONET || ret == EAGAIN) {
                        SLEEP_RETRY(err_ret, ret, retry, retry);
                } else
                        GOTO(err_ret, ret);
        }

        ret = dispatch_sysstat(&stat, force);
        if (ret)
                GOTO(err_ret, ret);

        printf("site:%d,%d\n"
               "rack:%d,%d\n"
               "node:%d,%d\n"
               "disk:%d,%d\n", 
               stat.site_online, stat.site_total,
               stat.rack_online, stat.rack_total,
               stat.node_online, stat.node_total,
               stat.disk_online, stat.disk_total);

        return 0;
err_ret:
        return ret;
}

static int __lich_config_dump()
{
        int ret;
        char hostname[MAX_NAME_LEN];

        ret = conf_init();
        if (ret)
                GOTO(err_ret, ret);

        dbg_sub_init();

        ret = net_gethostname(hostname, MAX_NAME_LEN);
        if (ret) {
                if (ret == ECONNREFUSED)
                        strcpy(hostname, "N/A");
                else
                        GOTO(err_ret, ret);
        }

        printf("globals.clustername:%s\n"
               "globals.hostname:%s\n"
               "globals.home:%s\n"
               "globals.localize:%s\n"
               "globals.wmem_max:%llu\n"
               "globals.rmem_max:%llu\n"
               "globals.replica_max:%u\n"
               "globals.crontab:%s\n"
               "globals.nohosts:%s\n"
               "globals.cgroup:%s\n"
               "globals.cleanlogcore:%s\n"
               "globals.testing:%s\n"
               "globals.default_protocol:%s\n"
               "metadata.meta:%d\n"
               "iscsi.iqn:%s\n"
               "iscsi.port:%d\n"
               "iscsi.vip:%s\n"
               "iscsi.timeout:%d\n"
               "cdsconf.disk_keep:%llu\n"
               "cdsconf.use_wlog:%d\n",
               gloconf.cluster_name,
               hostname,
               gloconf.home,
               gloconf.localize ? "on" : "off",
               (LLU)gloconf.wmem_max,
               (LLU)gloconf.rmem_max,
               LICH_REPLICA_MAX,
               gloconf.crontab ? "on" : "off",
               gloconf.nohosts ? "on" : "off",
               gloconf.cgroup ? "on" : "off",
               gloconf.cleanlogcore ? "on" : "off",
               gloconf.testing ? "on" : "off",
               gloconf.default_protocol,
               mdsconf.meta,
               sanconf.iqn,
               sanconf.iscsi_port,
               sanconf.iscsi_vip,
               sanconf.iscsi_timeout,
               (LLU)cdsconf.disk_keep,
               cdsconf.use_wlog);

        return 0;
err_ret:
        return ret;
}

static int __lich_hosts_dump()
{
        int ret;

        ret = conf_init();
        if (ret)
                GOTO(err_ret, ret);

        ret = hosts_init();
        if (ret)
                GOTO(err_ret, ret);

        hosts_dump();

        return 0;
err_ret:
        return ret;
}

int main(int argc, char *argv[])
{
        int ret, op = OP_NULL, force = 0;
        char c_opt;
        char *name = NULL;

        dbg_info(0);

        (void) verbose;

        while (srv_running) {
                int option_index = 0;

                static struct option long_options[] = {
                        { "setmeta", required_argument, 0, 0},
                        { "dropmeta", required_argument, 0, 0},
                        { "load", no_argument, 0, 0},
                        { "force", no_argument, 0, 0},
                        { "create", required_argument, 0, 0},
                        { "sysstat", no_argument, 0, 0},
                        { "configdump", no_argument, 0, 0},
                        { "hostsdump", no_argument, 0, 0},
                        { "castoff", required_argument, 0, 0},
                        { "setvip", no_argument, 0, 0},
                        { "createcluster", required_argument, 0, 'c'},
                        { "listnode", no_argument, 0, 'l'},
                        { "stat", required_argument, 0, 's'},
                        { "addnode", no_argument, 0, 'a'},
                        { "dropnode", no_argument, 0, 'd'},
                        //{ "movemeta", required_argument, 0, 'm'},
                        { "verbose", 0, 0, 'v' },
                        { "help",    0, 0, 'h' },
                        { 0, 0, 0, 0 },
                };

                c_opt = getopt_long(argc, argv, "cls:advh", long_options, &option_index);
                if (c_opt == -1)
                        break;

                switch (c_opt) {
                case 0:
                        switch (option_index) {
                        case 0:
                                DBUG("setmeta %s\n", optarg);
                                op = OP_SETMETA;
                                name = optarg;
                                break;
                        case 1:
                                DBUG("dropmeta %s\n", optarg);
                                op = OP_DROPMETA;
                                name = optarg;
                                break;
                        case 2:
                                __load__ = 1;
                                break;
                        case 3:
                                force = 1;
                                break;
                        case 4:
                                DBUG("create cluster\n");
                                op = OP_CREATE;
                                name = optarg;
                                break;
                        case 5:
                                op = OP_SYSSTAT;
                                break;
                        case 6:
                                op = OP_CONFIGDUMP;
                                break;
                        case 7:
                                op = OP_HOSTSDUMP;
                                break;
                        case 8:
                                op = OP_CASTOFF;
                                name = optarg;
                                break;
                        case 9:
                                op = OP_SETVIP;
                                break;
                        default:
                                fprintf(stderr, "Hoops, wrong op got xxxx !\n");
                        }

                        break;
                case 'c':
                        DBUG("create cluster\n");
                        op = OP_CREATE;
                        name = optarg;
                        break;
                case 'l':
                        DBUG("node list\n");
                        op = OP_LISTNODE;
                        break;
                case 's':
                        DBUG("stat %s\n", optarg);
                        op = OP_STAT;
                        name = optarg;
                        break;
                case 'a':
                        DBUG("addnode %s\n", optarg);
                        op = OP_ADDNODE;
                        break;
                case 'd':
                        DBUG("dropnode %s\n", optarg);
                        op = OP_DROPNODE;
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

        ret = env_init_simple("lich.admin");
        if (ret)
                GOTO(err_ret, ret);

        if (op != OP_NULL) {
                switch (op) {
                case OP_CREATE:
                        ret = cluster_create(name);
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                case OP_STAT:
                        ret = __admin_stat(name);
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                case OP_SETMETA:
                        ret = cluster_setmeta(name, force);
                        if (ret) {
                                sleep(3);
                                if (!(__is_meta(name))) {
                                        GOTO(err_ret, ret);
                                }
                        }
                        break;
                case OP_DROPMETA:
                        ret = cluster_dropmeta(name, force);
                        if (ret)
                                GOTO(err_ret, ret);

                        break;
                case OP_LISTNODE:
                        ret = __admin_listnode();
                        if (ret)
                                GOTO(err_ret, ret);

                        break;
                case OP_ADDNODE:
                        ret = cluster_addnode((void *)&argv[2], argc - 2);
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                case OP_DROPNODE:
                        ret = __admin_dropnode((void *)&argv[2], argc - 2);
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                case OP_CASTOFF:
                        ret = __admin_castoff(name);
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                case OP_SETVIP:
                        ret = __admin_setvip();
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                case OP_SYSSTAT:
                        ret = __admin_sysstat(force);
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                case OP_CONFIGDUMP:
                        ret = __lich_config_dump();
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                case OP_HOSTSDUMP:
                        ret = __lich_hosts_dump();
                        if (ret)
                                GOTO(err_ret, ret);
                        break;
                default:
                        usage();
                        EXIT(EINVAL);
                }
        }

        return 0;
err_ret:
        exit(ret);
}
