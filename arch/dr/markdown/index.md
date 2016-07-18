# 目录

1. [场景分析](#场景分析)
2. [HA and CDP](#HA and CDP)
3. [技术方案](#技术方案)

# 场景分析

## 基本的备份/恢复过程

- 备份/切换
- 回切 (暂不支持)

备份五要素:

- 源
- 目标
- 通路
- 引擎
- 策略

## 复制元数据，做到对应用层完全透明

- 卷属性
- 精简配置

## 一致性卷组(Consistent Group)

在虚机挂载多个卷的情况下，需要保证多个卷的数据一致性，首先，对多个相关卷打快照，然后复制到远程站点。

## 应用层的数据一致性

如果虚机已切换到DR站点，而卷或一致性卷组还没有同步到DR站点，应该如何处理？

## 支持复杂架构

- 两地三中心

# HA and CDP

<!--
![可用性指数图](../images/availability.jpeg)
-->

设计选项：

- 同构和异构
- 快照和commit log
- 同步复制和异步复制

相关特性：

- 消重
- 压缩
- 加密
- 纠删码

衡量指标：

- RPO (Recovery Point Object, 可以容忍的数据丢失量)
- RTO (从故障开始到恢复所需的时间, 是一个综合性指标)

# 技术方案

一期实现目标：

> 根据远程复制策略，复制选择的卷的快照到远程DR站点，保证做到*快照级别*的可恢复性和业务连续性。

## 基本假设和设计原则

- 底层数据通路是IP可达
- 满足RPO和RTO规格
- 分离策略和机制

## 架构

### 远程复制一个卷的过程

    def sync_volume():
        while True:
            create_snapshot()
            sync_data()
            sync_metadata()

    def read_snapshot_chunk_or_cow():
        found = False
        with chunk.lock():
            found = find_in_snapshot(chunk)
            if found:
                read_only()
            else:
                cow_to_temp()
                sync_data_from_temp()
        if found:
            sync_data()
        else:
            sync_data_from_temp()
               
       

## 实现

### 断点续传
### 禁止master volume接收备份数据

## 评估

一是实现功能要求，而是满足质量属性。

### 性能

# 参考系统

- EMC RecoveryPoint
- VMWare VSAN
- CEPH
- 飞康
