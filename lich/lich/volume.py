#!/usr/bin/env python2
# -*- coding: utf-8 -*-


import utils

from umptypes import UmpPath

from base import LichBase
from runner import local_runner


def _remove_attr_exc_handler(e, *args, **kw):
    ignoreNoKey = kw.get('ignoreNoKey')
    if ignoreNoKey and "Required key not available" in str(e):
        pass
    else:
        raise Exception(e)


class LichVolumeParam(object):
    def __init__(self, path, size=None, protection_domain=None):

        self.path = path
        self.size = size
        self.protection_domain = protection_domain


class LichVolume(LichBase):

    @local_runner()
    def create(self, param):
        path = param.path
        cmd = '%s create %s --size %sB -p %s' % (self.lichbd, path.long_volume_name, param.size, path.protocol)
        return cmd

    @local_runner()
    def delete(self, param):
        path = param.path
        cmd = '%s rm %s -p %s' % (self.lichbd, path.long_volume_name, path.protocol)
        return cmd

    @local_runner()
    def info(self, param):
        path = param.path
        cmd = '%s info %s -p %s' % (self.lichbd, path.long_volume_name, path.protocol)
        return cmd

    @local_runner()
    def list(self, param):
        path = param.path
        cmd = '%s ls %s -p %s' % (self.lichbd, path.long_pool_name, path.protocol)
        return cmd

    @local_runner()
    def resize(self, param):
        path = param.path
        cmd = '%s resize %s --size %sB -p %s' % (self.lichbd, path.long_volume_name, param.size, path.protocol)
        return cmd

    @local_runner()
    def rename(self, param, new_name):
        path = param.path
        cmd = '%s rename %s %s/%s -p %s' % (self.lichbd, path.long_volume_name, path.long_pool_name, new_name, path.protocol)
        return cmd

    @local_runner()
    def set_attr(self, param, attr, value):
        path = param.path
        return "%s --attrset %s %s '%s'" % (self.lichfs, path.volume_path, attr, value)

    @local_runner()
    def get_attr(self, param, attr):
        path = param.path
        return "%s --attrget %s %s" % (self.lichfs, path.volume_path, attr)

    @local_runner(exc_handler=_remove_attr_exc_handler)
    def remove_attr(self, param, attr, ignoreNoKey=False):
        path = param.path
        return "%s --attrremove %s %s" % (self.lichfs, path.volume_path, attr)

    @local_runner()
    def _iscsi_connection(self, param):
        path = param.path
        cmd = '%s --connection %s' % (self.lich_inspect, path.volume_path)
        return cmd

    def iscsi_connection(self, param):
        res = self._iscsi_connection(param)
        return utils.parse_connection(res)

    def list_snapshots(self, param):
        path = param.path
        raise NotImplementedError

    @local_runner()
    def flatten(self, param):
        path = param.path
        return "%s --flat %s" % (self.lich_snapshot, path)


if __name__ == '__main__':
    path = UmpPath('pool1/vol15')
    param = LichVolumeParam(path, size=1)
    vol = LichVolume()
    vol.create(param)
    vol.set_attr(param, 'attr1', 'value1')
    vol.get_attr(param, 'attr1')
    vol.remove_attr(param, 'attr1')
    print vol.list(param)
    print vol.info(param)
    param.size = 2
    vol.resize(param)
    print vol.info(param)
    vol.delete(param)
