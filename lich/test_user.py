#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import unittest


from fusionstor import (
    UserManager,
)

from base import TestBase


class TestAll(TestBase):
    def setUp(self):
        super(TestAll, self).setUp()

        self.user_m = UserManager()

        self._delete('user001')

        ret, resp = self.user_m.create(name='user001', identity='tenancy', password='mdsmds', quota=0, repnum=3,
                                       protection_domain_ids='1')
        self.assertEqual(resp.status_code, 200)
        records = resp.records
        if records:
            self.user_id = records['id']

    def tearDown(self):
        self._delete('user001')
        self._delete('user002')
        self._delete('user003')

    def test_create_dup_user(self):
        self._delete('user002')

        ret, resp = self.user_m.create(name='user002', identity='tenancy', password='mdsmds', quota=0, repnum=3,
                                       protection_domain_ids='1')
        self.assertEqual(resp.status_code, 200)

        ret, resp = self.user_m.create(name='user002', identity='tenancy', password='mdsmds', quota=0, repnum=3,
                                       protection_domain_ids='1')
        self.assertEqual(resp.status_code, 500)

        self._delete('user002')

    def test_delete(self):
        self._delete('user003')

        ret, resp = self.user_m.create(name='user003', identity='tenancy', password='mdsmds', quota=0, repnum=3,
                                       protection_domain_ids='1')
        records = resp.records
        if records:
            self.user_id = records['id']

        ret, resp = self.user_m.delete(self.user_id)
        self.assertEqual(resp.status_code, 200)

        ret, resp = self.user_m.delete(self.user_id)
        self.assertNotEqual(resp.status_code, 200)

        self._delete('user003')

    def test_user_stat(self):
        ret, resp = self.user_m.stat(self.user_id)
        self.assertEqual(resp.status_code, 200)

    def test_list(self):
        self._test_list(self.user_m, find_id=self.user_id, 
                fields=['sp_repnum', 'sp_quota', 'sp_pdomains'])

    def test_login(self):
        # _, resp = self.user_m.login()
        pass

    def _delete(self, name):
        exists = self.find(self.user_m, name)
        if exists:
            self.user_m.delete(exists['id'])


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestAll]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    unittest.main()
