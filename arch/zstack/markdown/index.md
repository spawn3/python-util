% HCI系统BOOTSTRAP过程
% 兼顾ZSTACK管理节点的HA
% 2016.08.16

-----

# 快速过程

1. 制作ZSTACK镜像(VirtualBox, KVM)
1. 下载ZSTACK镜像，创建并启动VM01
1. 通过ZSTACK@VM01，部署并初始化S100存储系统
1. 导入ZSTACK镜像到S100存储系统, 创建并启动VM02
1. 同步ZSTACK数据：从VM01到VM02
1. 控制权切换到VM02

# 过程详解

目标：

- 简化HCI部署过程，争取做到集中式一键完成
- 在S100里托管ZSTACK管理节点的HA

## 制作ZSTACK镜像

首先，需要制作包含ZSTACK全部安装包的镜像，该镜像有两个用途：

- 创建VM01: 在主控节点上，用于部署阶段的ZSTACK@VM01 (VirtualBox)
- 创建VM02: 导入S100存储系统后，用于生产环境下的ZSTACK@VM02 (KVM)

## 创建VM01 

可以使用一款笔记本作为主控节点，继续后面的操作。

在主控节点上安装VirtualBox虚拟化软件，以先前制作的ZSTACK镜像来创建虚拟机VM01。

## 通过ZSTACK@VM01, 部署并启动S100存储系统

启动ZSTACK@VM01服务后，按正常流程部署并初始化S100存储系统。

准备多个S100节点的过程：

1. 安装操作系统
1. 配置网络

目前，通过手工方式完成以上步骤，可以进一步简化以上过程：采取IPMI方式或PXE方式。

## 创建VM02

待S100存储集群启动后，导入第一步制作的ZSTACK镜像到S100存储系统，并创建VM02。

## 同步数据到VM02

把ZSTACK数据从VM01同步到VM02。

> 注意：该过程只能执行一次，要有机制避免因为多次同步数据，导致线上数据受损。

## 切换到VM02

同步数据成功后，关闭VM01，后续即可通过ZSTACK@VM02管理HCI集群。

# 其它事项

## 容错处理

可能发生的故障：

- S100无法启动
- VM02无法启动

## 备份ZSTACK数据

 
<!--

![可用性指数图](images/availability.jpeg)

<div align=center>
<img src="../images/availability.jpeg" width="300" height="200" alt="可用性" align="center" />
<img src='../images/availability.jpeg' style='width:640px;height=480px'>
</div>

-->
