#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest


class TestPool(unittest.TestCase):
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_stat(self):
        self.assertEqual(0, 1)

    def test_list(self):
        self.assertEqual(1, 1)


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestPool]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    t = unittest.TextTestRunner()
    t.run(suite())
