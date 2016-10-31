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

    def test_create_snapshot(self):
        self._del_snapshot(self.snap_name)
        self._create_snapshot(self.snap_name)
        self._del_snapshot(self.snap_name)

    @unittest.skipIf(False, 'skip this')
    def test_delete_volume_with_snapshot(self):
        """a volume cannot be deleted if there is a snapshot created from it.
        """

        self._del_snapshot(self.snap_name)
        self._create_snapshot(self.snap_name)
        self._del_volume(self.volume_name, status_code=500)
        self._del_snapshot(self.snap_name)

        if not self.exists(self.volume_m, self.volume_name):
            self._create_volume(self.volume_name, self.size)

    def _change_path(self, path):
        idx = path.find(':')
        if idx != -1:
            path = path[idx+1:]
        return path

    def test_list(self):
        ret, resp = self.volume_m.list(skip=0, limit=100)
        found = False
        if resp.records:
            self.assertEqual(resp.status_code, 200)
            for item in resp.records:
                try:
                    path = self._change_path(item['path'])
                    if path == self.volume_name:
                        found = True
                    self.volume_m.stat(path)
                except ValueError, e:
                    print e

        self.assertEqual(found, True)

    @unittest.skipIf(False, 'skip this')
    def test_nofound(self):
        _, resp = self.volume_m.stat(self.notfound_volume_name)
        self.assertEqual(resp.status_code, 404)

    @unittest.skipIf(False, 'skip this')
    def test_resize(self):

        size = 2 * 1024**3

        _, resp = self.volume_m.resize(self.volume_name, size)

        self.assertEqual(resp.status_code, 200)

        _, resp = self.stat_volume(self.volume_name)
        self.assertEqual(resp.records['size'], size)


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestAll]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    unittest.main()
