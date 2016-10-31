#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import utils
from umptypes import UmpPath

from base import LichBase
from runner import local_runner


def _delete_exc_handler(e, *args, **kw):
    if str(e).find("Required key not available") != -1:
        pass
    else:
        raise Exception(e)


class LichSnapshotParam(object):
    def __init__(self, path, cluster_id=1, host_ip=None):
        self.path = path


class LichSnapshot(LichBase):

    @local_runner()
    def create(self, param):
        path = param.path
        return "%s --create %s" % (self.lich_snapshot, path.snap_path)

    @local_runner(exc_handler=_delete_exc_handler)
    def delete(self, param):
        path = param.path
        return "%s --remove %s" % (self.lich_snapshot, path.snap_path)

    def remove(self, param):
        return self.delete(param)

    @local_runner()
    def rollback(self, param):
        path = param.path
        return "%s --rollback %s" % (self.lich_snapshot, path.snap_path)

    @local_runner()
    def clone(self, param, new_vol_path):
        path = param.path
        return  "%s --clone %s %s" % (self.lich_snapshot, path.snap_path, new_vol_path)

    @local_runner()
    def flat(self, param):
        path = param.path
        return "%s --flat %s" % (self.lich_snapshot, path)

    @local_runner()
    def protect(self, param, on=True):
        """ lich.snapshot --protect snap_path 0|1.
        :param param:
        :param on:
        :return:
        """
        path = param.path
        cmd = 'protect'
        status = 1 if on else 0
        return "%s --%s %s %s" % (self.lich_snapshot, cmd, path.snap_path, status)

    # @local_runner()
    def unprotect(self, param):
        return self.protect(param, on=False)

    @local_runner()
    def _list(self, param):
        path = param.path
        return "%s --list %s" % (self.lich_snapshot, path)

    def list(self, param):
        res = self._list(param)
        return utils.split_lines(res)


if __name__ == '__main__':
    path = UmpPath('/pool1/vol11@snap01')
    param = LichSnapshotParam(path)
    snap = LichSnapshot()
    snap.create(param)
    snap.protect(param)
    print snap.list(param)
    snap.unprotect(param)
    snap.remove(param)
