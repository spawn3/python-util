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
        self.sname_01 = UmpPath('snap.a/b@c')
        self.clone_01 = UmpPath('snap.a/d')

        self.size = 1024*1024*1024
        self.new_size = 2 * self.size

        # clean
        self.del_volume(self.clone_01)
        self.del_snapshot(self.sname_01)
        self.del_volume(self.vname_01)
        self.del_pool(self.pname_01)

        self.create_pool(self.pname_01)
        self.create_volume(self.vname_01, self.size)
        self.create_snapshot(self.sname_01)

    def tearDown(self):
        self.del_volume(self.clone_01)
        self.del_snapshot(self.sname_01)
        self.del_volume(self.vname_01)
        self.del_pool(self.pname_01)

    def test_create(self):
        pass


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestAll]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    unittest.main()
