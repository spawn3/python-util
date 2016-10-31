#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import json
import traceback
import sys
from pprint import pprint



class LichSetting(object):
    def __init__(self):
        self.lich_home = '/opt/fusionstack'


class LichBase(object):
    def __init__(self):
        self.settings = LichSetting()
        self.lich_home = self.settings.lich_home

        # commands
        self.lich = '%s/lich/admin/cluster.py' % self.lich_home
        self.lich_cluster = '%s/lich/admin/cluster.py' % self.lich_home
        self.lich_node = '%s/lich/admin/node.py' % self.lich_home
        self.lichfs = '%s/lich/libexec/lichfs' % self.lich_home
        self.lichbd = '%s/lich/libexec/lichbd' % self.lich_home
        self.lich_snapshot = '%s/lich/libexec/lich.snapshot' % self.lich_home
        self.lich_inspect = '%s/lich/libexec/lich.inspect' % self.lich_home
        self.lich_admin = '%s/lich/libexec/lich.admin' % self.lich_home
        self.lich_license = '%s/lich/libexec/lich.license' % self.lich_home
        self.lich_syncump = '%s/lich/admin/syncump.py' % self.lich_home


if __name__ == '__main__':
    lich = LichBase()
