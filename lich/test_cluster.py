#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest


from fusionstor import (
    ClusterManager,
    PDomainManager,
)

from base import TestBase


class TestAll(TestBase):
    def setUp(self):
        super(TestAll, self).setUp()

        self.cluster_m = ClusterManager()
        self.pd_m = PDomainManager()

    def tearDown(self):
        pass

    def test_list(self):
        _, resp = self.cluster_m.list(skip=0, limit=100)
        self.assertEqual(resp.status_code, 200)
        if resp.records:
            self.assertEqual(len(resp.records), 1)
            for item in resp.records:
                try:
                    self.assertIn('config_str', item)
                    self.assertIn('iqn2', item)
                    self.assertIn('vip', item)

                    _id = item['id']
                    disk_total = item['disk_total']
                    disk_used = item['disk_used']

                    self.assertEqual(_id, 1)
                    self.assertGreater(disk_total, disk_used)

                    self.cluster_m.stat(_id)
                except ValueError, e:
                    print e

    def test_pdomain(self):
        _, resp = self.pd_m.list(skip=0, limit=100)
        self.assertEqual(resp.status_code, 200)
        if resp.records:
            self.assertEqual(len(resp.records), 1)
            for item in resp.records:
                try:
                    #self.assertIn('cluster_id', item)

                    _id = item['id']
                    disk_total = item['disk_total']
                    disk_used = item['disk_used']

                    self.assertEqual(_id, 1)
                    self.assertGreater(disk_total, disk_used)

                    self.pd_m.stat(_id)
                except ValueError, e:
                    print e

def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestAll]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    unittest.main()
