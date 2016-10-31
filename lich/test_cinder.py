#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest


from fusionstor import (
    UserManager,
)

from base import TestBase


class TestAll(TestBase):
    def setUp(self):
        super(TestAll, self).setUp()

        self.user_m = UserManager()

        user = self.find(self.user_m, 'cinder')
        if not user:
            _, resp = self.user_m.create(name='cinder', identity='tenancy', password='mdsmds', quota=None, repnum=None)
            self.assertEqual(resp.status_code, 200)
            user = resp.records
        self.user_id = user['id']

    def tearDown(self):
        #_, resp = self.user_m.delete(self.user_id)
        #self.assertEqual(resp.status_code, 200)
        pass

    def test_list(self):
        super(TestAll, self)._test_list(self.user_m, self.user_id)

    def test_login(self):
        # _, resp = self.user_m.login()
        pass


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestAll]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    unittest.main()
