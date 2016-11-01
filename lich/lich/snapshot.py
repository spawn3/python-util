#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import json

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

    def remove(self, path):
        return self.delete(path)

    @local_runner()
    def _list(self, path):
        return "%s --list %s -n" % (self.lich_snapshot, path.volume_path)

    def list(self, path):
        rc, res = self._list(path)
        if rc == 0 and res:
            res = json.loads(res[0])
        else:
            res = {}
        return rc, res

    def stat(self, path):
        rc, snaptree = self.list(path)
        snaptree = snaptree.get('snapshot', {})
        return rc, self._find_snap(snaptree, path.snap_name)

    def children(self, path):
        rc, info = self.stat(path)
        if info and 'child' in info:
            return info.get('child', {}).keys()
        return []

    def _find_snap(self, snaptree, snap):
        for k, v in snaptree.iteritems():
            if k == snap:
                return v
            elif 'child' in v:
                info = self._find_snap(v['child'], snap)
                if info:
                    return info
        return None

    def exists(self, path):
        rc, info = self.stat(path)
        return rc == 0 and info

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


if __name__ == '__main__':
    path = UmpPath('pool1/volume1@snap01')
    snap = LichSnapshot()
    print snap.list(path)

    # snap.create(path)
    # snap.protect(path)
    # snap.unprotect(path)
    # snap.remove(path)
