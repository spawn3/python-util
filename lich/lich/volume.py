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


class LichVolume(LichBase):

    @local_runner()
    def create(self, path, size):
        cmd = '%s create %s --size %sB -p %s' % (self.lichbd, path.long_volume_name, size, path.protocol)
        return cmd

    @local_runner()
    def delete(self, path):
        cmd = '%s rm %s -p %s' % (self.lichbd, path.long_volume_name, path.protocol)
        return cmd

    @local_runner()
    def stat(self, path):
        cmd = '%s info %s -p %s' % (self.lichbd, path.long_volume_name, path.protocol)
        return cmd

    @local_runner()
    def exists(self, path):
        raise NotImplementedError

    @local_runner()
    def list(self, path):
        cmd = '%s ls %s -p %s' % (self.lichbd, path.long_pool_name, path.protocol)
        return cmd

    @local_runner()
    def resize(self, path, size):
        cmd = '%s resize %s --size %sB -p %s' % (self.lichbd, path.long_volume_name, size, path.protocol)
        return cmd

    @local_runner()
    def rename(self, path, new_name):
        cmd = '%s rename %s %s/%s -p %s' % (self.lichbd, path.long_volume_name, path.long_pool_name, new_name, path.protocol)
        return cmd

    @local_runner()
    def set_attr(self, path, attr, value):
        return "%s --attrset %s %s '%s'" % (self.lichfs, path.volume_path, attr, value)

    @local_runner()
    def get_attr(self, path, attr):
        return "%s --attrget %s %s" % (self.lichfs, path.volume_path, attr)

    @local_runner(exc_handler=_remove_attr_exc_handler)
    def remove_attr(self, path, attr, ignoreNoKey=False):
        return "%s --attrremove %s %s" % (self.lichfs, path.volume_path, attr)

    @local_runner()
    def _iscsi_connection(self, path):
        cmd = '%s --connection %s' % (self.lich_inspect, path.volume_path)
        return cmd

    def iscsi_connection(self, path):
        res = self._iscsi_connection(path)
        return utils.parse_connection(res)

    def list_snapshots(self, path):
        raise NotImplementedError

    @local_runner()
    def flatten(self, path):
        return "%s --flat %s" % (self.lich_snapshot, path)


if __name__ == '__main__':
    path = UmpPath('pool1/vol15')
    vol = LichVolume()
    vol.create(path, 1)
    vol.set_attr(path, 'attr1', 'value1')
    vol.get_attr(path, 'attr1')
    vol.remove_attr(path, 'attr1')
    print vol.list(path)
    print vol.info(path)
    vol.resize(path, 2)
    print vol.info(path)
    vol.delete(path)
