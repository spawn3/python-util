#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest

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


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestAll]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    unittest.main()
