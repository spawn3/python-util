# RAFT

Replicated state machine

可以提交与真正提交不同。可以提交的必要条件是entry被写入多数派的日志。

如何处理log不一致的清空？从leader向follow单向同步log entry，直到完全一致。

## 规则

### 选举规则

leader必须包含所有committed entries。

log里没有提交的entry，如何处理？

log会清空，snapshot

### 提交规则

写入多数派是必要非充分条件，new entry是否可committed，分两种情况：
- current term
- older term

## 故障处理

leader故障

follower故障

分区
