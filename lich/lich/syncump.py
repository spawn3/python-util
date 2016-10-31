#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from base import LichBase, RemoteLocation
from runner import http_runner


class LichSyncumpParam(RemoteLocation):
    def __init__(self, host_ip=None):
        super(LichSyncumpParam, self).__init__(host_ip)


class LichSyncump(LichBase):

    @http_runner()
    def get_network(self, param):
        cmd = '%s get_network ' % (self.lich_syncump)
        return cmd
    
    @http_runner()
    def iscsi_port(self, param, port="check"):
        cmd = "%s iscsi_port --port %s" % (self.lich_syncump, port)
        return cmd

    @http_runner()
    def eth_hosts(self, param, host=None):
        host = host or param.host_ip
        cmd = "%s etc_hosts --host %s" % (self.lich_syncump, host)
        return cmd



if __name__ == '__main__':
    param = LichSyncumpParam(host_ip='192.168.120.211', dev_name='/dev/vda')
    node = LichSyncump()
    # print disk.add(param)
