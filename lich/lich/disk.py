#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from base import LichBase, RemoteLocation
from runner import http_runner


class LichDiskParam(RemoteLocation):
    def __init__(self, host_ip=None, cluster_id=1, dev_name=None):
        super(LichDiskParam, self).__init__(host_ip=host_ip, cluster_id=cluster_id)
        self.dev_name = dev_name

    def get_names(self, sep=' '):
        if isinstance(self.dev_name, list):
            return sep.join(self.dev_name)
        else:
            return self.dev_name


class LichDisk(LichBase):

    @http_runner()
    def add(self, param, is_force=False):
        cmd = '%s --disk_add %s ' % (self.lich_node, param.dev_name)
        if is_force:
            cmd += '--force'
        return cmd

    @http_runner()
    def delete(self, param):
        cmd = '%s --disk_del %s ' % (self.lich_node, param.dev_name)
        return cmd

    @http_runner()
    def list(self, param):
        cmd = '%s --disk_list --json' % (self.lich_node)
        return cmd

    @http_runner()
    def add_raid(self, param):
        cmd = '%s --disk_list ' % (self.lich_node)
        return cmd

    @http_runner()
    def delete_raid(self, param):
        cmd = '%s --disk_list ' % (self.lich_node)
        return cmd
    
    @http_runner()
    def raid_missing(self, param):
        cmd = '%s --raid_miss' % (self.lich_node)
        return cmd
    
    @http_runner()
    def light(self, param, op, device, serial_number=None):
        cmd = '%s --disk_light %s %s' % (self.lich_node, op, device)
        if serial_number:
            cmd = '%s --disk_light %s %s' % (self.lich_node, op, serial_number)
        return cmd


if __name__ == '__main__':
    param = LichDiskParam(host_ip='192.168.120.211', dev_name='/dev/vda')
    disk = LichDisk()
    print disk.list(param)
    # print disk.add(param)
