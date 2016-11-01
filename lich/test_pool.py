#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest

import utils
from base import TestBase


class TestAll(TestBase):
    def setUp(self):
        super(TestAll, self).setUp()

        self.pool_name = 'pool.a'
        self.volume_name = 'pool.a/b'
        self.size = utils.GB(1)

        self._del_volume(self.volume_name)
        self._del_pool(self.pool_name)

    def tearDown(self):
        # self._del_volume(self.volume_name)
        # self._del_pool(self.pool_name)
        pass

    @unittest.skipIf(False, 'skip this')
    def test_delete(self):
        self._create_pool(self.pool_name)
        self._del_pool(self.pool_name)

    def test_delete_if_volume(self):
        self._create_pool(self.pool_name)

        self._create_volume(self.volume_name, self.size)
        # 2: No such file or directory
        # 39: Directory not empty
        self._del_pool(self.pool_name, status_code=39)
        self._del_volume(self.volume_name)

        self._del_pool(self.pool_name)

    def test_stat(self):
        self._create_pool(self.pool_name)
        self.stat_pool(self.pool_name)
        self._del_pool(self.pool_name)
        self.stat_pool(self.pool_name)

    def test_list(self):
        self._create_pool(self.pool_name)
        pools = self.list_pools()
        self.assertIn(self.pool_name, pools)
        self._del_pool(self.pool_name)
        pools = self.list_pools()
        self.assertNotIn(self.pool_name, pools)


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    s.addTests(load_from(TestAll))

    return s


if __name__ == '__main__':
    unittest.main()
