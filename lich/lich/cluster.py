#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from base import LichBase, RemoteLocation
from runner import http_runner
import utils
import config


class LichClusterParam(RemoteLocation):
    def __init__(self, host_ip=None, cluster_id=1):
        super(LichClusterParam, self).__init__(host_ip=host_ip, cluster_id=cluster_id)


class LichCluster(LichBase):
    def create(self, param):
        pass

    def delete(self, param):
        pass

    @http_runner()
    def _health(self, param):
        cmd = '%s health' % (self.lich)
        return cmd

    def health(self, param):
        s = self._health(param)
        return utils.parse_health(s)

    @http_runner()
    def fsstat(self, param):
        cmd = '%s --fsstat' % (self.lichfs)
        return cmd

    @http_runner()
    def _dump_config(self, param):
        cmd = '%s --configdump' % (self.lich_admin)
        return cmd

    def dump_config(self, param): 
        s = self._dump_config(param)
        return utils.parse_global_config(s)

    @http_runner()
    def _listnode(self, param):
        cmd = '%s listnode' % self.lich
        return cmd

    def listnode(self, param):
        res = self._listnode(param)
        return utils.parse_listnode(res) 

if __name__ == '__main__':
    param = LichClusterParam(host_ip=config.host_ip)
    obj = LichCluster()
    res = obj.health(param)
    print
    print res
