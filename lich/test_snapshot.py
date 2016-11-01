#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest

from lich.umptypes import UmpPath
from base import TestBase


class TestAll(TestBase):
    def setUp(self):
        super(TestAll, self).setUp()

        self.pname_01 = UmpPath('snap.a')
        self.vname_01 = UmpPath('snap.a/b')

        self.sname_01 = UmpPath('snap.a/b@s1')
        self.sname_02 = UmpPath('snap.a/b@s2')
        self.sname_03 = UmpPath('snap.a/b@s3')
        self.sname_04 = UmpPath('snap.a/b@s4')

        self.clone_01 = UmpPath('snap.a/d')

        self.size = 1024*1024*1024
        self.new_size = 2 * self.size

        # clean
        self.del_volume(self.clone_01)

        self.del_snapshot(self.sname_01)
        self.del_snapshot(self.sname_02)
        self.del_snapshot(self.sname_03)
        self.del_snapshot(self.sname_04)

        self.del_volume(self.vname_01)
        self.del_pool(self.pname_01)

        self.create_pool(self.pname_01)
        self.create_volume(self.vname_01, self.size)

    def tearDown(self):
        # self.del_volume(self.clone_01)
        self.del_volume(self.vname_01)
        self.del_pool(self.pname_01)

    @unittest.skipIf(False, 'skip me')
    def test_create(self):
        self.create_snapshot(self.sname_01)

        rc, res = self.lich_snapshot.stat(self.sname_01)
        self.assertIsNotNone(res)

        self.del_snapshot(self.sname_01)

    def _assertSnap(self, snap):
        rc, res = self.lich_snapshot.stat(snap)
        self.assertIsNotNone(res)

    @unittest.skipIf(False, 'skip me')
    def test_multi_create(self):
        """
        s1 -> s2 -> s3

        :return:
        """
        snaps = [self.sname_01, self.sname_02, self.sname_03]
        for snap in snaps:
            self.create_snapshot(snap)
            rc, res = self.lich_snapshot.stat(snap)
            self.assertIsNotNone(res)

        for snap in snaps:
            self.del_snapshot(snap)
            rc, res = self.lich_snapshot.stat(snap)
            self.assertIsNone(res)

    @unittest.skipIf(False, 'skip me')
    def test_rollback(self):
        """
        s1 -> s2 -> s3
               | -> s4

        :return:
        """
        self.create_snapshot(self.sname_01)
        self.create_snapshot(self.sname_02)
        self.create_snapshot(self.sname_03)

        self.lich_snapshot.rollback(self.sname_02)

        self.create_snapshot(self.sname_04)

        self._assertSnap(self.sname_01)
        self._assertSnap(self.sname_02)
        self._assertSnap(self.sname_03)
        self._assertSnap(self.sname_04)

        child = self.lich_snapshot.children(self.sname_01)
        self.assertListEqual(child, [self.sname_02.snap_name])

        child = self.lich_snapshot.children(self.sname_02)
        self.assertListEqual(child, [self.sname_03.snap_name, self.sname_04.snap_name])

        self.del_snapshot(self.sname_01)
        self.del_snapshot(self.sname_02)
        self.del_snapshot(self.sname_03)
        self.del_snapshot(self.sname_04)


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestAll]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    unittest.main()
