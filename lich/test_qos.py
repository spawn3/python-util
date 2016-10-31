#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest


from fusionstor import (
    QosManager,
)

from base import TestBase

class TestAll(TestBase):
    def setUp(self):
        super(TestAll, self).setUp()

        self.qos_01 = 'qos_04'

        self.find_and_delete(self.qos_m, self.qos_01)

        _, resp = self.qos_m.create(name=self.qos_01, username='cinder', throt_avg=1, throt_burst_max=2, throt_burst_time=3)
        self.assertEqual(resp.status_code, 200)
        records = resp.records
        if records:
            self.qos_id = records['id']

    def tearDown(self):
        print 'qos_id', self.qos_id
        _, resp = self.qos_m.delete(self.qos_id)
        self.assertEqual(resp.status_code, 200)

    def test_update(self):
        _, resp = self.qos_m.update(self.qos_id, name='qos_00', throt_avg=2, throt_burst_max=4, throt_burst_time=4)
        self.assertEqual(resp.status_code, 200)
        records = resp.records
        if records:
            self.assertEqual(records['id'], self.qos_id)
            self.assertEqual(records['name'], 'qos_00')
            self.assertEqual(records['throt_burst_max'], '4')

    def test_list(self):
        _, resp = self.qos_m.list(skip=0, limit=100)
        found = False
        if resp.records:
            self.assertEqual(resp.status_code, 200)
            for item in resp.records:
                try:
                    _id = item['id']
                    if _id == self.qos_id:
                        found = True
                    self.qos_m.stat(_id)
                except ValueError, e:
                    print e

        self.assertEqual(found, True)


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestAll]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    unittest.main()
