#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest

from pprint import pprint


from fusionstor import (
    PoolManager,
    VolumeManager,
    SnapshotManager,
)

from base import TestBase


class TestAll(TestBase):
    def setUp(self):
        super(TestAll, self).setUp()

        self.pool_name = 'volume.a'
        self.volume_name = 'volume.a/b'
        self.snap_name = 'volume.a/b@c'
        self.size = 1024*1024*1024
        self.notfound_volume_name = 'volume.a/c'

        self._del_snapshot(self.snap_name)
        self._del_volume(self.volume_name)
        self._del_pool(self.pool_name)

        self._create_pool(self.pool_name)
        self._create_volume(self.volume_name, self.size)

    def tearDown(self):
        self._del_snapshot(self.snap_name)
        self._del_volume(self.volume_name)
        self._del_pool(self.pool_name)


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestAll]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    unittest.main()
