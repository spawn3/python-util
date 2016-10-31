#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest


from fusionstor import (
    TokenManager,
)

from base import TestBase


class TestAll(TestBase):
    def setUp(self):
        self.token_m = TokenManager()

        #user = self.find(self.token_m, 'user001')
        #if user:
        #    _, resp = self.token_m.delete(user['id'])
        #    self.assertEqual(resp.status_code, 200)

        ret, resp = self.token_m.create()
        self.assertEqual(resp.status_code, 200)
        records = resp.records
        if records:
            self.token_id = records['id']

    def tearDown(self):
        #: token can be deleted after a period of time
        # _, resp = self.token_m.delete(self.token_id)
        # self.assertEqual(resp.status_code, 200)
        pass

    def test_create(self):
        """Create another token after creating the first token.
        Both use the same argument,and the ids should be the same.
        """
        ret, resp = self.token_m.create()
        self.assertEqual(resp.status_code, 200)
        records = resp.records
        if records:
            self.assertEqual(self.token_id, records['id'])

    def test_list(self):
        # TODO self.token_id maybe be deleted on expire
        self._test_list(self.token_m, find_id=None, 
                fields=['id', 'user_id', 'token'])

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
