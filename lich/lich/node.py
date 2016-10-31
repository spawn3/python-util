#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from base import LichBase, RemoteLocation
from runner import http_runner


class LichNodeParam(RemoteLocation):
    def __init__(self, host_ip=None, cluster_id=1, dev_name=None):
        super(LichNodeParam, self).__init__(host_ip=host_ip, cluster_id=cluster_id)
        self.dev_name = dev_name

    def get_names(self, sep=' '):
        if isinstance(self.dev_name, list):
            return sep.join(self.dev_name)
        else:
            return self.dev_name


class LichNode(LichBase):

    @http_runner()
    def start(self, param):
        cmd = '%s --start ' % (self.lich_node)
        return cmd

    @http_runner()
    def delete(self, param):
        cmd = '%s --disk_del %s ' % (self.lich_node, param.dev_name)
        return cmd

    @http_runner()
    def list(self, param):
        cmd = '%s --disk_list ' % (self.lich_node)
        return cmd

    @http_runner()
    def add_raid(self, param):
        cmd = '%s --disk_list ' % (self.lich_node)
        return cmd

    @http_runner()
    def delete_raid(self, param):
        cmd = '%s --disk_list ' % (self.lich_node)
        return cmd


if __name__ == '__main__':
    param = LichNodeParam(host_ip='192.168.120.211', dev_name='/dev/vda')
    node = LichNode()
    # print disk.add(param)
