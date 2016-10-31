#!/usr/bin/env python2
#-*- coding: utf-8 -*-


import sys
import datetime
import json


reload(sys)
sys.setdefaultencoding('utf-8')


TipDict = {
    # NORMAL
    0: '异常',

    1: '名字已存在',
    2: '参数错误',

    100: '数据库操作失败',

    # Physical
    200: '集群不存在',
    201: '集群中没有节点',
    202: '集群中没有可连接节点',

    # Logical
    300: '存储池不存在',
    301: '存储池非空',

    400: '卷不存在',

    500: '快照不存在',
    501: '快照处于保护模式',
}


TIPS = {
    'InvalidParameter': '无效参数',
    'DBError': '数据库错误',
    'NotFound': '没有发现资源',
    'PoolNotFound': '存储池不存在',
    'PoolNotEmpty': '存储池非空',
}


class UmpException(Exception):
    def __init__(self, *args, **kw):
        cls_name = self.__class__.__name__
        name = TIPS.get(cls_name, cls_name)
        if args:
            name = '%s: %s' % (name, args[0])
        if kw:
            name = '%s: %s' % (name, json.dumps(kw))
        super(UmpException, self).__init__(name)


class CheckFunNotDefined(UmpException):
    pass


class DBError(UmpException):
    pass


class NameExists(UmpException):
    pass


class NotFound(UmpException):
    pass


class AlreadyExists(UmpException):
    pass


class ReferencedError(UmpException):
    pass


class InvalidParameter(UmpException):
    pass


class StatusError(UmpException):
    pass


class QuotaError(UmpException):
    pass


## ---------------------------------------------------------

class TokenNotfound(NotFound):
    pass


class TokenExpire(UmpException):
    pass


class TokenError(UmpException):
    pass


class LichFault(Exception):
    pass


class LichLiscenFault(LichFault):
    pass


class AuthenticationFailed(UmpException):
    pass


class PermissionDenied(UmpException):
    pass


class IpRangeError(UmpException):
    pass


class HostUnable(UmpException):
    pass


class HostNotEmpty(UmpException):
    pass


class HostnameDuplica(UmpException):
    pass


class ImageUnable(UmpException):
    pass


class ResourcesOver(UmpException):
    '''资源跨集群'''
    pass


class ResourcesNotFound(UmpException):
    pass


class ResourcesTypeError(UmpException):
    '''资源类型错误'''
    pass


class ResourcesInuse(UmpException):
    '''资源正在使用中'''
    pass


class ResourcesConditionDeficit(UmpException):
    '''资源条件不具备 '''
    pass


class OperateReject(UmpException):
    '''操作被拒绝'''
    pass


class UnknownParameter(UmpException):
    '''不能处理的参数'''
    pass


class DeleteError(UmpException):
    pass


class Duplica(UmpException):
    pass


class InvalidPath(InvalidParameter):
    pass


# CLUSTER

class ClusterNotFound(NotFound):
    pass


class ClusterDuplica(UmpException):
    pass


class ClusterDoubleIp(UmpException):
    pass


class ClusterNotEmpty(UmpException):
    pass


# Protection Domain

class PDomainNotFound(NotFound):
    pass


# Protocol

class ProtocolNotSupported(NotFound):
    pass


# USER

class UserNotFound(NotFound):
    pass


class UserPasswordError(UmpException):
    pass


# POOL

class VpoolNotEmpty(UmpException):
    pass


class VpoolUnable(UmpException):
    pass


class VpoolConditionDeficit(UmpException):
    pass


class PoolNotFound(NotFound):
    pass


class PoolFound(AlreadyExists):
    pass


class PoolQuotaError(QuotaError):
    pass


class PoolNotEmpty(UmpException):
    pass


# VOLUME

class VolumeNotFound(NotFound):
    pass


class VolumeFound(AlreadyExists):
    pass


class VolumeInuse(UmpException):
    pass


class VolumesTooMany(UmpException):
    pass


class VolumeLocked(UmpException):
    pass


class VolumeError(UmpException):
    pass


class VolumeReferenced(ReferencedError):
    pass


# SNAPSHOT

class SnapshotNotFound(NotFound):
    pass


class SnapshotFound(AlreadyExists):
    pass


class SnapshotProtected(StatusError):
    pass


class SnapshotReferenced(ReferencedError):
    pass


# VGROUP

class VGroupNotFound(NotFound):
    pass


class VGroupReferenced(ReferencedError):
    pass


# CGSNAPSHOT

class CGSnapshotNotFound(NotFound):
    pass


if __name__ == '__main__':
    import traceback
    try:
        raise NameExists(a=1)
    except Exception, e:
        traceback.print_exc()
