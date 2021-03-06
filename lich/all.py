#!/usr/bin/env python2
# -*- coding: utf-8 -*-


import unittest

import test_pool
import test_volume
import test_snapshot


class TestPool(unittest.TestCase):
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_stat(self):
        self.assertEqual(0, 0)

    def test_list(self):
        self.assertEqual(1, 1)


if __name__ == '__main__':
    t = unittest.TextTestRunner()

    t.run(test_pool.suite())
    t.run(test_volume.suite())
    t.run(test_snapshot.suite())
