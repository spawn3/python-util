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


class LichSnapshot(LichBase):

    @local_runner()
    def create(self, path):
        return "%s --create %s" % (self.lich_snapshot, path.snap_path)

    @local_runner(exc_handler=_delete_exc_handler)
    def delete(self, path):
        return "%s --remove %s" % (self.lich_snapshot, path.snap_path)

    @local_runner()
    def stat(self, path):
        raise NotImplementedError

    @local_runner()
    def exists(self, path):
        raise NotImplementedError

    def remove(self, path):
        return self.delete(path)

    @local_runner()
    def rollback(self, path):
        return "%s --rollback %s" % (self.lich_snapshot, path.snap_path)

    @local_runner()
    def clone(self, path, new_vol_path):
        return  "%s --clone %s %s" % (self.lich_snapshot, path.snap_path, new_vol_path)

    @local_runner()
    def flat(self, path):
        return "%s --flat %s" % (self.lich_snapshot, path)

    @local_runner()
    def protect(self, path, on=True):
        """ lich.snapshot --protect snap_path 0|1.
        :path path:
        :path on:
        :return:
        """
        cmd = 'protect'
        status = 1 if on else 0
        return "%s --%s %s %s" % (self.lich_snapshot, cmd, path.snap_path, status)

    # @local_runner()
    def unprotect(self, path):
        return self.protect(path, on=False)

    @local_runner()
    def _list(self, path):
        return "%s --list %s" % (self.lich_snapshot, path)

    def list(self, path):
        res = self._list(path)
        return utils.split_lines(res)


if __name__ == '__main__':
    path = UmpPath('pool1/vol11@snap01')
    snap = LichSnapshot()
    snap.create(path)
    snap.protect(path)
    print snap.list(path)
    snap.unprotect(path)
    snap.remove(path)
