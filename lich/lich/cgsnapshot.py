#!/usr/bin/env python2
# -*- coding: utf-8 -*-


import utils
from base import LichBase, RemoteLocation
from runner import http_runner

from Ump.objs.root import init_root


def _delete_exc_handler(e, *args, **kw):
    if str(e).find("Required key not available") != -1:
        pass
    else:
        raise Exception(e)


class LichCGSnapshotParam(RemoteLocation):
    def __init__(self, host_ip=None, vols=None, snap_name=None, protocol='iscsi', cluster_id=1):
        """
        vols: [pool1/vol11, pool1/vol12]
        """
        super(LichCGSnapshotParam, self).__init__(host_ip, cluster_id=cluster_id)

        cluster = init_root().cluster

        self.protocol = protocol
        self.protocol_root = cluster.protocol_root(protocol)
        self.vols = vols
        self.snap_name = snap_name

    def expand_vols(self, sep=',', vols=None):
        if isinstance(vols, list) and vols:
            tmp = vols
        else:
            tmp = self.vols
        vols = ['/%s/%s' % (self.protocol_root, vol) for vol in tmp]
        return sep.join(vols)


class LichCGSnapshot(LichBase):

    @http_runner()
    def create(self, param):
        vol_path = param.expand_vols()
        return "%s --create %s@%s" % (self.lich_snapshot, vol_path, param.snap_name)

    @http_runner(exc_handler=_delete_exc_handler)
    def delete(self, param):
        vol_path = param.expand_vols()
        return "%s --remove %s@%s" % (self.lich_snapshot, vol_path, param.snap_name)

    def remove(self, param):
        return self.delete(param)

    @http_runner()
    def rollback(self, param):
        vol_path = param.expand_vols()
        return "%s --rollback %s@%s" % (self.lich_snapshot, vol_path, param.snap_name)

    @http_runner()
    def clone(self, param, new_vols=None):
        vol_path = param.expand_vols()
        new_vol_path = param.expand_vols(vols=new_vols)
        return  "%s --clone %s@%s %s" % (self.lich_snapshot, vol_path, param.snap_name, new_vol_path)

    @http_runner()
    def protect(self, param, on=True):
        vol_path = param.expand_vols()
        cmd = 'protect' if on else 'unprotect'
        return "%s --%s %s@%s" % (self.lich_snapshot, cmd, vol_path, param.snap_name)

    @http_runner()
    def unprotect(self, param):
        return self.protect(param, on=False)

    @http_runner()
    def _list(self, param):
        vol_path = param.expand_vols()
        return "%s --list %s" % (self.lich_snapshot, vol_path)

    def list(self, param):
        res = self._list(param)
        return utils.split_lines(res)

    @http_runner()
    def flat(self, param):
        vol_path = param.expand_vols()
        return "%s --flat %s@%s" % (self.lich_snapshot, vol_path, param.snap_name)


if __name__ == '__main__':
    param = LichCGSnapshotParam(host_ip='192.168.120.211',
                               vols=['pool1/vol11', 'pool1/vol12'],
                               snap_name='snap01',
                               protocol='iscsi')
    group = LichCGSnapshot()
