% LICH存储系统待实现功能
%
% 2016.08.16

-----

# 项目计划

## 问题

LICH相关:

- list pool的扩展性问题
- 一致性组的远程复制
- 主机映射关系的实现可能有问题？
- iSCSI CHAP认证

管理平台：

- CMDB的维护
- Protection Domain
- 多租户
- 引入多租户后，系统依赖于CMDB，CLI是不是需要重写？
- RESTFUL API (refer CEPH)
- Openstack Cinder Driver
- cinder与多租户有无交叉影响？

QoS:

- Pool
- Volume
- Host Mapping

##

物理资源：

- 仪表盘（资源概览）
- 引入保护域（存储池）
- 主机管理
- 磁盘管理
- 事件管理
- 报警管理
- 注册管理
- 系统设置
- 自动快照策略
- 一致性卷组
- 异步远程复制

逻辑资源：

- 引入多租户（资源池，卷等虚拟资源隶属于租户, 一个租户下面可以有多个用户）
- 资源池
- 卷管理
- 快照管理
- 访问策略
- QoS

存储策略：

- 副本数

ISCSI：

- linux iscsi client限制
- 访问策略（chap认证）
- 主机映射关系管理

OpenStack：

- cinder driver

SA:

- 配置管理 （导出/导入）
- 操作日志
- 事件/告警/通知
- 统计报表
- 性能管理
- 版本升级

高级功能：

- 自动快照策略
- 一致性卷组
- 异步远程复制

监控项：

- 系统级：

- 节点级：

 - cpu
 - memory
 - disk
 - network (ip, hostname)
 - lichd
 - disk/node down
 - smart reallocate (扇区坏，不可逆过程）

- 卷级：

 - chunk unavailable
 - IOPS

- Port：

 - 用于分析网络流量TCPDUMP, 用于诊断问题


事件有几类：

- syslog事件
- 触发器事件


# 功能

## 块服务

### 物理资源

- 节点
- 磁盘（分级, 缓存）
- 保护域
- 存储池
- 故障域

保护域是对集群的所有节点进行物理划分，一个节点只能属于一个保护域。
一个卷的所有数据只能属于一个保护域，保护域起到物理隔离的目的。

存储池进一步对保护域进行物理划分，限定了资源的数据范围和故障边界。

故障域：一个数据块的每一个副本，存放在不同的故障域里。即便某一个故障域发生了整体故障，
也不会影响到别的副本，保障了数据安全。一个保护域或存储池内，必须有足够的故障域，才能
保证数据块的所有副本，都能分配到存储位置。

对每个节点上的存储介质，包括SSD和HDD，可以分成多级进行组织，也可以拿出部分高速存储介质，
作为缓存层，提升系统的整体性能。

需要支持把资源池或卷绑定到特定的介质层上，如高IOPS，高吞吐量，大容量。

### 逻辑资源

- pool
- volume
- snapshot

pool是一逻辑实体，类似于文件夹或容器，具有如下特征：

- 当前的实现，pool可以嵌套
- pool可以指定副本策略，并具有继承关系
- pool无法指定容量
- pool可以指定保护域？
- pool可以指定介质层？
- QoS?


属性      描述  值范围
--------- ----- ----------
名字
创建时间
副本策略
容量上限
卷数上限?
IOPS
BW
Latency
--------- ----- ----------

volume是核心对象，有元数据和数据，具有如下特征：

- 隶属于pool，以pool为父
- volume可以指定副本策略，并具有继承关系
- volume有大小，并可以动态调整大小
- volume可以生成快照，也可以从快照clone出volume（可写快照）
- 支持精简配置
- 支持tiering
- QoS?

snapshot隶属于volume，是一种特殊的volume，具有如下特征：

- 以所在volume为父
- 只读
- rollback
- clone出卷
- 一个卷上的snapshot有上限
- 当前实现是线性的，不是树状的
- 创建snapshot极快，毫秒级

资源对象上可以设定扩展属性。

### 多租户

引入多租户以后，UMP就成为了lich系统不可或缺的一部分，而不再是可有可无。相关数据，也需要重点保护起来了。

多租户模式：支持服务质量QoS控制，提供针对某个特定用户卷的带宽与IOPS的限速机制。
同时还支持不同保护域的划分。

在创建租户时，可以指定一个或多个保护域，也可以不指定，在系统范围内分配存储空间。

用户可以创建资源池和逻辑卷，资源池和逻辑卷要能够绑定到特定保护域内。
不绑定的时候，系统在用户可达的保护域内自动分配存储空间。

租户可以共享底层存储池。

租户隔离机制：

- UI隔离：每一个租户有独立的管理界面，只能看到自己创建的资源池和卷
- 资源隔离：每一个租户无法访问其它租户的卷（iscsi情况下，通过chap认证方式进行隔离）
- QoS

<!--
数据消费层的一个重要组成部分是展现给用户/租户的门户（portal），也就是每个用户的管理平台（GUI/CLI）。
这是一个专属的环境，可以由每个用户自己动态定制，它的功能包括但不限于：
部署，监控，事件/警报管理，资源的使用，报表生成，流程管理等。
此外用户也可以在专属门户内定制所需的数据服务。
-->

### QoS

### 数据迁移和复制

在不同保护域内迁移或复制卷

复制关系（源卷，目标卷，策略，其它属性）CRUD

## 主机系统

linux下iscsi客户端的局限，导致无法实现系统最大卷数100000。

iscsi chap鉴权

主机映射关系

## 系统管理UMP

项目描述：

分布式存储系统管理，包括两部分：web UI和CLI。

主要功能模块列表：

- 物理资源管理（集群，保护域，故障域，节点，磁盘等）
- 逻辑资源管理（存储池，卷，快照，主机映射等）
- 基于角色的用户管理 (RBAC)
- 配置管理（导出/导入）
- 监控 (SNMP)
- 日志
- 事件
- 告警
- 统计报表
- 仪表盘
- 许可管理
- 版本管理
- 性能管理

监控项，触发器，事件，告警，通知


## 对接云平台

openstack cinder


# 问题


资源的owner是谁？访问控制策略是什么？

如果一个pool里有很多volume，如何列出？

远程复制

一致性组

# 其它

Resource   C   R   U   D
--------- --- --- --- ---
Pool
Volume
Snapshot
--------- --- --- --- ---

## 生命周期模型

在删除特定资源后，要回收资源所占资源。

属性：结构属性，策略属性和行为属性

## 存储池

属性：

- 创建时间
- 副本策略
- 卷数上限
- 容量上限
- IOPS/BW/Latency

操作：

- CRUD
- 扩展/缩小


## 逻辑卷

属性：

- 创建时间
- Pool
- 副本策略
- IOPS/BW/Latency

支持功能：

- 创建/初始化/卸载/删除/扩展
- 修改卷所在的池
- 查询卷的IOPS/BW/Latency
- QoS (IOPS)

## 快照

- 基于快照生成快照(clone)
- 回收快照空间

## 多主机映射

- 创建映射关系
- 删除映射关系
- 强制解除映射关系，中止连接
- 在卸载卷时，删除映射关系？


compression
de-duplication
multi-tenancy

- cli如何访问数据库？
- 相关信息附于attr？

## 数据恢复
