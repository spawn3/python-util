cmake_minimum_required(VERSION 2.8)

set(CMAKE_INSTALL_PREFIX "/opt/fusionstack")
project (lich4)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "-O3 -g -Wall -W -Werror -Werror=return-type -rdynamic -D_GNU_SOURCE -D_REENTRANT")
set(CMAKE_CXX_FLAGS "-O3 -g -Wall -W -Werror -Werror=return-type -rdynamic -D_GNU_SOURCE -D_REENTRANT")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/build
    ${CMAKE_CURRENT_SOURCE_DIR}/config/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/include
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/include
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/include
    ${CMAKE_CURRENT_SOURCE_DIR}/nbd/include
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/include
    ${CMAKE_CURRENT_SOURCE_DIR}/sheepdog/include
    )
add_library(lich4 SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/build/confl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/build/confy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/config/lib/configure.c

    ${CMAKE_CURRENT_SOURCE_DIR}/schedule/schedule.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schedule/cpuset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schedule/vm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schedule/core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schedule/corenet.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schedule/corerpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schedule/corenet_maping.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schedule/corenet_connect.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schedule/aio.c

    ${CMAKE_CURRENT_SOURCE_DIR}/prof/prof_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/prof/prof_net.c
    ${CMAKE_CURRENT_SOURCE_DIR}/prof/prof_vm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/prof/prof_lichbd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/prof/prof_dio.c

    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/mem_cache.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/gettime.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/mem_hugepage.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/job_dock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/cmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/crc32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/crcrs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/daemon.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/dbg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/hash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/hash_table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/htable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/cache.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/journal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/lock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/mem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/path.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/stat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/skiplist.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/sysutil.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/timer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/xdr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/ylog.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/ytime.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/bmap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/bitmap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/md5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/squeue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/analysis.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/bh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/longtask.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/fnotify.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/worker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/yid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/tpool.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/base64.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ylib/lib/array_table.c

    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/sock/sock_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/sock/sock_passive.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/sock/sock_tcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/sock/sock_unix.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/sock/sock_udp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/sock/sock_xnect.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/sock/sock_xmit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/sock/sdevent.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/net_crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/main_loop.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/net_events.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/net_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/net_passive.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/net_table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/heartbeat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/xnect.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/net_xmit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/net_msg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/net_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/fence.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/arp_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/arp_host.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/hosts.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/net_tool.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/net_vip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/net/net_vip_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/rpc/minirpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/rpc/rpc_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/rpc/msgqueue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/rpc/rpc_proto.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/rpc/rpc_passive.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/rpc/rpc_reply.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/rpc/rpc_request.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ynet/rpc/rpc_table.c

    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/node/node.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/node/nodectl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/node/paxos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/node/paxos_srv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/node/paxos_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/node/node_srv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/node/node_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/node/env.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/cluster/cluster.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/cluster/cluster_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/cluster/dlock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/cluster/maping.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/cluster/network.c

    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/dispatch/dispatch_srv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/dispatch/dispatch_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/dispatch/dispatch.c

    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/rootable/rootable_dt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/rootable/rootable_srv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/rootable/rootable_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/rootable/rootable.c

    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/msgqueue/mq_master.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/msgqueue/mq_slave.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/msgqueue/mq_event.c

    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/nodepool/nodetable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/nodepool/diskmap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster/nodepool/nodeid.c

    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/volume.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/vnode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/stor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/stor_root.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/rmvol_bh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/system.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/table_proto.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/table_xattr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/table_snap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/volume_proto.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/table1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/table2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/volume_proto_io.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/volume_proto_analysis.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/volume_proto_snapshot.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/pool_proto.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/pool_rename.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/pool_rmvol.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/stor_ctl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/volume_bh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/pool_ctl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/volume_ctl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/recovery.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/castoff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/balance.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/balance_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/lease_ctl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/controller/lease.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/locator_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/stor_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/md.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/md_root.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/md_map.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/md_chunk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/storage/md_parent.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/disk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/disk_hsm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/diskmd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/disk_aio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/disk_mt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/replica_srv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/replica_write.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/replica_read.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/replica_sync.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/replica_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/replica_hsm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/clock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/clock_mem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/clock_merge.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/replica/wlog.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/chunk/chunk_cleanup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/chunk/chunk_bh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/chunk/chunk_sync.c
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/chunk/chunk_proto.c

    ${CMAKE_CURRENT_SOURCE_DIR}/nbd/src/nbd_srv.c

    ${CMAKE_CURRENT_SOURCE_DIR}/sheepdog/src/sheepdog.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sheepdog/src/sd_map.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lichbd/lichbd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lichbd/lichbd_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/src/sunrpc_passive.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/src/sunrpc_reply.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/src/sunrpc_proto.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/src/attr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/src/error.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/src/readdir.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/src/nfs_events.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/src/mountlist.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/src/nfs_state_machine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/src/xdr_nfs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/src/nfs_proc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nfs/src/nfs_srv.c

    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/block.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/iscsid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/iscsi_mem_cache.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/digest.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/ua.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/iscsi_md5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/tio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/config.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/lich_io.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/sha1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/conn.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/chap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/session.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/target_disk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/iscsi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/param.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/target.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/vol.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/iscsi_worker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/debug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iscsi/src/cmds.c
    )
target_link_libraries(lich4 aio)

add_executable(lichd ${CMAKE_CURRENT_SOURCE_DIR}/lichd/src/main.c)
target_link_libraries(lichd pthread aio crypt crypto uuid ssl rt sqlite3 m lich4)


install(TARGETS lich4 LIBRARY DESTINATION lich/lib)
install(TARGETS lichd RUNTIME DESTINATION lich/sbin)
