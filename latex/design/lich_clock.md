# 原理

## 比较现有数据一致性方案

vctl包含若干chunk，chunk对应若干replica。为了保证数据一致性，需要满足下列条件：
- vctl满足唯一性要求(通过lease实现)
- replica满足一致性和透明性要求(通过owner,clock,dirty,magic实现)

对任一chunk，需要满足：
- 在epoch内，clock连续

正常情况下，login、io等过程应该不需要过多check。可以放宽vctl的一致性要求，
允许多个leader存在，而不破坏safety规则。

故障条件下，某些状态发生变化。当vctl检测到这些变化时，发起peering/check过程。

故障类型：
- IO Error
- 单节点故障导致admin切换
- 单节点故障导致vctl切换
- 单节点掉电导致clock丢失
- 集群整体掉电导致所有clock丢失
- 网络分区

现有方案导致如下问题：
- 为了避免lease过期，到处加入renew逻辑，且无法保证vctl的唯一性
- 控制器切换面临ABA问题
- lease timeout导致IO中断时间过长
- chunk check影响IO性能
- 节点级故障恢复需要扫描所有chunk，更好的做法是定向扫描
- 磁盘级故障恢复利用了磁盘的sqlite记录

ceph的pg对应于vctl，object对应于chunk。pg的属性包括：epoch，acting\_set, up\_set等。pg的恢复过程：peering，recovery，backfill。
osd通过heartbeat方式诊断与别的osd的连通性，并把结果上报给monitor。monitor维护有cluster map。
cluster map的变化，会导致pg到osd映射的变化，从而触发peering过程。

epoch: 加载卷不改变，migrate才改变，add 1。用于区分vctl发生变化的情况。或者时每次加载卷成功后+1。
epoch变更时，可reset clock。

## VFM2

dev\_vfm1更新思路如下描述
- vfm：Volume Fail Map，是指在volume看来，哪些节点出现了故障。这里，vfm不是数据一致性标记，而是过滤标记，vfm上的chunk"疑似"不一致。vfm在整个流程中最小化使用，完全保留了原有的一致性算法，可以用"#define ENABLE_VFM 0"来完全关闭。当vfm关闭或者vfm“满”的时候，fallback成原有的一致性算法
- vfm位置。vfm记录在subvol上，每16GB数据一个vfm，这样做的好处是粒度适当，既不过大也不过小
- vfm满的问题。vfm最多可以记录32条记录。但是vfm上记录的条目所在的故障域的个数，不能超过福本数，比如有三个rack，rack1,rack2,rack3。三个副本，最坏情况下，vfm记录的节点，不能超出两个rack。
- consistent定义。consistent是指数据是一致的，即元数据不更新的情况下，可以对数据进行读写而不影响一致性。即使实际情况下有数据离线，或者不一致。
- intact定义。intact是指数据是完备的，所有副本都在线，并且完全相同。intact是consistent的一个特例
- chunk\_proto\_consistent函数，源自原来的chunk\_proto\_needcheck，改动很小，不过结果取反了。增加了离线时候的判断，如果离线副本在vfm中，则认为这个副本是一致的
- vfm标记。__chunk_clean函数，针对clean副本的检查，如果离线，则尝试加入vfm，加入之前判断，如果无法加入，fallback到原有的模式
- 不一致检查。在vfm存在的情况下，直接通过节点是否在线来判断一致性的方式失效。需要结合vfm来判断。volume_ctl_chunk_unintact提供了一个迭代器，扫描所有unintact的chunk
- 恢复过程的vfm过滤。隐藏在__chunk\_get\_clean1/__chunk_get_clean2，原则很简单，阻止在vfm上的chunk成为主副本
- 数据恢复。数据恢复可以和过去一致，但在恢复结束后需要清空vfm。调用volume_ctl_vfm_cleanup来执行。原则上来讲，最优的恢复方法是每个subvol为单位恢复，然后清空vfm。

## 名词定义:

volume controller（vctl）: 卷控制器负责卷io的读写，以及raw副本位置管理，以及卷快照管理

vg: 卷组, 为了减轻etcd的负担，通过把卷hash到一个vg中，由vg来维护vfm, 这样就要求通一个vg的卷的元数据会分布到同一组节点内, 每次卷迁移必须要以组为单位进行迁移，也就是说同一个vg内的vctl需要在任何时间都要在同一个节点上   

peering: 这个名字来自于ceph，vctl首次加载或者vctl迁移时,或者节点离线又上线时,当访问每个chunk之前需要先做时钟同步，检查副本的版本是否一致，如果不一致需要做副本同步.

logic clock: 逻辑时钟是vctl的状态以及以及从vctl看到节点或者磁盘拓扑变化的一个全局顺序,隐含了vctl的lease功能,逻辑时钟的值由vfm中的版本号来给出.

etcd: 用来存放lich的一些元数据(vfm, metadata tree, vg，iscsi卷到fid的映射)以及选举功能，后续有可能把volume的chunkinfo也放到etcd里边

vfm: volume fault node map, vctl在运行时标记diskid离线的表,每次卷迁移以及节点或者磁盘离线都由vctl或者vg来更新vfm,  同时由vfm来为卷控制器提供逻辑时钟 , vfm存放于etcd中.当vfm中有数据时表示volume处于降级模式.

admin: 通过etcd选举出来，负责lich集群的vip管理，diskmap管理，添加删除节点或者磁盘, 当某个节点宕机时间超过阈值,admin节点会将这个节点提出集群，并更新diskmap的版本号.

diskmap: 是一个全局的逻辑时钟，这个逻辑时钟只是用来管理新节点的加入和删除节点, 集群内的节点没定期同步这个逻辑时钟，如果发生改变就要启动节点级balance.

nodemap: 每个节点维护一个自己的整个集群的nodeid表，这个是个局部的逻辑时钟，是每个节点看到的集群状态, 由网络层来维护,vfm依赖nodemap来标记节点离线.
如果出现本节点到某个节点心跳超时的次数达到阈值则在nodemap标记对应节点状态为down,nodemap的版本号+1, 并根据diskmap来更新nodeid表的成员.


## 超时的定义:

1 心跳超时: 在规定的时间内没收到对方的心跳请求或者自己的请求没有返回，即判定一次心跳超时.(500)

2 宕机判定超时: 如果超过规定的时间内心跳一直超时即可判定对方宕机, 一旦出现宕机超时，会触发所有的rpc任务超时返回，并作相应recover处理.(1500)

6 iscsi inititor端的超时设定，需大于io超时，以免触发的linux scsi层的io超时，引起iscsi recovery(hdd:6000, ssd:2000).

2 rpc超时: 如果在规定的时间内rpc操作没有从远端返回，同时没有出现宕机超时，即可判定rpc超时，如果是io由vctl标记vfm磁盘dirty.(hdd:6000, ssd:1000).

4 磁盘io超时: 本地磁盘超时，这个超时时间应该小于rpc超时.(hdd:5000, ssd:1000)

5 更新/获取 vfm超时,根据etcd官方文档里边的写延迟，可设定vfm超时为100ms.

以上时间值的单位都是毫秒,rpc超时除了io可以不触发任何操作直到宕机超时时再再做相应动作,对于协程的超时时间都可以在调用yield()时设定，而无需在其他地方来判断超时

#  iscsi login过程:

admin节点维护vip ，所以所有的login请求都要经过admin节点, 而admin通过某种方式（etcd或者pool controller,这个取决于后续的实现）获取到vctl的位置然后重定向到vctl所在的节点.
vctl所在节点的tgt通过vctl层逻辑用lun名字来获取fid，
vctl会查询自身cache是否有lun名字对应的key，如果没有vctl层会向etcd来获取vfm,同时把vfm版本号+1，并向etcd更新vfm.

1,admin会通过本机nodemap来确定哪一个vctl的副本是在线的，然后选取一个在线的vctl返回给客户端,并不需要将在线的vctl放在chunkinfo的第一位,所以也就避免向etcd或者poolcontroller更新.
2,这个Login只涉及到单卷Login的过程，后续如果要实现卷组的话，需要考虑怎么确保同一个卷组内卷login到同一个节点
.


#  时间管理
#  网络层处理逻辑
#  心跳算法
#  节点管理
#  io流程
#  io recover
