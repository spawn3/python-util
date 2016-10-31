#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest


from fusionstor import (
    PoolManager,
    VolumeManager,
    SnapshotManager,
)


from base import TestBase

class TestAll(TestBase):
    def setUp(self):
        super(TestAll, self).setUp()

        self.pname_01 = 'snap.a'
        self.vname_01 = 'snap.a/b'
        self.sname_01 = 'snap.a/b@c'
        self.clone_01 = 'snap.a/d'

        self.size = 1024*1024*1024
        self.new_size = 2 * self.size

        # clean
        self._del_volume(self.clone_01)
        self._del_snapshot(self.sname_01)
        self._del_volume(self.vname_01)
        self._del_pool(self.pname_01)

        self._create_pool(self.pname_01)
        self._create_volume(self.vname_01, self.size)
        self._create_snapshot(self.sname_01)

    def tearDown(self):
        self._del_volume(self.clone_01)
        self._del_snapshot(self.sname_01)
        self._del_volume(self.vname_01)
        self._del_pool(self.pname_01)

    def test_create(self):
        pass

    def test_create_clone(self):
        self._del_volume(self.clone_01)
        ret, resp = self.snapshot_m.clone(self.sname_01, self.clone_01)
        self.assertEqual(resp.status_code, 200)
        self._del_volume(self.clone_01)

    @unittest.skipIf(True, 'skip this')
    def test_delete_snap_with_clone(self):
        """a snapshot cannot be deleted if there is a clone gotten from it.
        """
        self._del_volume(self.clone_01)
        self.snapshot_m.clone(self.sname_01, self.clone_01)
        self._del_snapshot(self.sname_01, status_code=500)
        self._del_volume(self.clone_01)

        if not self.exists(self.snapshot_m, self.sname_01):
            self._create_snapshot(self.sname_01)

    @unittest.skipIf(False, 'skip this')
    def test_protect(self):
        ret, resp = self.snapshot_m.protect(self.sname_01, is_protect=True)
        self.assertEqual(resp.status_code, 200)

        self._del_snapshot(self.sname_01, status_code=500)

        ret, resp = self.snapshot_m.protect(self.sname_01, is_protect=False)
        self.assertEqual(resp.status_code, 200)

        if not self.exists(self.snapshot_m, self.sname_01):
            self._create_snapshot(self.sname_01)

    def _change_path(self, path):
        idx = path.find(':')
        if idx != -1:
            path = path[idx+1:]
        return path

    @unittest.skipIf(False, 'skip this')
    def test_nofound(self):
        #_, resp = self.snapm.stat(self.sname_01)
        #self.assertEqual(resp.status_code, 404)
        pass

    @unittest.skipIf(False, 'skip this')
    def test_list(self):
        ret, resp = self.snapshot_m.list(skip=0, limit=100)
        self.assertEqual(resp.status_code, 200)


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestAll]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    unittest.main()
