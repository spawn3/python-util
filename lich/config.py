#!/usr/bin/env python2
# -*- coding: utf-8 -*-


class VolumeConfig(object):
    def __init__(self, name, snapshots=['snap_01'], cloned_volumes=['clone_01']):
        self.name = name
        self.snapshots = snapshots
        self.cloned_volumes = cloned_volumes

        self.size = 1024*1024*1024
        self.new_size = 2 * self.size

    @property
    def pool_name(self):
        idx = self.name.find('/')
        return self.name[:idx]


VOLUMES = [
    VolumeConfig('a/b')
]
