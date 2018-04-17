# Consensus Problem

## PAXOS

## RAFT

Replicated state machine

模拟完整的运行过程，跟踪每个节点的状态变化。在此过程中，模拟各种故障场景，以及如何应对。

Index定义了时序，引入term是为了处理并发和异常情况。每个Index，对应paxos instance。

选举和提交都是按多数派原则进行，并加入额外规则。

可以提交与真正提交不同。可以提交的必要条件是entry被写入多数派的日志。

如何处理log不一致的清空？从leader向follow单向同步log entry，直到完全一致。

读和写不同，分区情况下会出现stale read问题。

## 规则

### 选举规则

leader必须包含所有committed entries。

log里没有提交的entry，如何处理？

log会清空，snapshot

### 提交规则

写入多数派是必要非充分条件，new entry是否可committed，分两种情况：
- current term
- older term (某index的值一旦提交，就不能被更大term的值overwrite)

在比较新旧时，term具有更高优先级。

log可以想象为一个巨大的数组，其中每一项都有index。通过commitIndex分为两部分：已提交的和未提交的。
commitIndex不需持久化，leader的log就是真理，单向同步leader log到各个followers上。

如何保证entry是连续地apply到SM上的？且每一index对应的值/command执行iff一次且仅有一次。

## 故障处理

何时会出现多主？出现多主影响写和读操作的safety吗？
- 断开又连接
- 分区

leader故障

follower故障

## LICH

lich复制组的拓扑结构不同于raft，pg的与raft类似。

lease机制为了保证vctl的唯一性，如vctl不能满足唯一性，通过多个vctl向同一chunk并发写入时，会破坏数据一致性。
能否引入新的条件，去掉lease机制带来的复杂性?

通过HB检测节点故障，通过IO Error检测到磁盘故障

checklist：
- chunk端检查：offline, reset(ltime)，replica status，op
- replica端检查：owner, magic, clock。（何时更新owner和magic？）

owner是vctl的nid，magic是连接建立时生成的。当vctl切换后，nid可能变化，也可能不变化（ABA问题），magic是随机数，有很大的概率会变化。
虽然可以通过owner，magic区分vctl发生切换了这样的事件，但这样的组合是无序的，无法比较新旧。

clock类似于raft的index，表示log order。clock由vctl维护，按io到达的顺序，传播到各个副本上。副本按clock order连续写入，不允许有空洞。
clock单调递增，在第一次load时，需要从replica上同步最新的clock值。

clock丢失的情况：当个节点或集群

disk fault：mark check
node fault：offline、ltime

定义commit和return的语义。没有返回成功的io，系统状态是不确定的，或为A或为B，须满足原子性要求。

各个副本只有clock，若clock相等，则认为副本相等。选择哪个副本作为权威副本，都能满足一致性要求。

因为副本上没有记录log，所以不能按snapshot+log的方式进行同步，而是直接采用了image。
任选其一，覆盖其余。

sync和io操作互斥。在sync的过程中，io挂起。

## CEPH
