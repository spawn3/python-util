#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from base import LichBase, RemoteLocation
from runner import http_runner


class LichInspectParam(RemoteLocation):
    def __init__(self, host_ip=None):
        super(LichInspectParam, self).__init__(host_ip)


class LichInspect(LichBase):

    @http_runner()
    def list(self, param):
        cmd = '%s --disk_list' % (self.lich_node)
        return cmd


if __name__ == '__main__':
    param = LichInspectParam(host_ip='192.168.120.211')
    obj = LichInspect()
    obj.list(param)
    # print disk.add(param)
