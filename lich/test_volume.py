#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest
from pprint import pprint


from lich.umptypes import UmpPath

from base import TestBase


class TestAll(TestBase):
    def setUp(self):
        super(TestAll, self).setUp()

        self.pool_name = UmpPath('volume.a')
        self.volume_name = UmpPath('volume.a/b')
        self.size = 1024*1024*1024

        self.del_volume(self.volume_name)
        self.del_pool(self.pool_name)

        self.create_pool(self.pool_name)
        # self.create_volume(self.volume_name, self.size)

    def tearDown(self):
        # self.del_volume(self.volume_name)
        self.del_pool(self.pool_name)
        pass

    def test_delete(self):
        self.create_volume(self.volume_name, self.size)
        self.del_volume(self.volume_name)

    def test_stat(self):
        self.create_volume(self.volume_name, self.size)
        self.stat_volume(self.volume_name)
        self.del_volume(self.volume_name)


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestAll]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    unittest.main()
