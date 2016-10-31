#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest

import utils
from base import TestBase


from fusionstor import (
    PoolManager,
    VolumeManager,
    SnapshotManager,
    UserManager
)


class TestAll(TestBase):
    def setUp(self):
        super(TestAll, self).setUp()

        self.pool_name = 'pool.a'
        self.volume_name = 'pool.a/b'
        self.size = utils.GB(1)

        self._del_volume(self.volume_name)
        self._del_pool(self.pool_name)

        ret, resp = self._create_pool(self.pool_name)
        records = resp.records
        if records:
            self.assertEqual(self.pool_name, self._change_path(records['path']))

    def tearDown(self):
        # self._del_volume(self.volume_name)
        self._del_pool(self.pool_name)

    @unittest.skipIf(True, 'skip this')
    def test_delete_pool_with_volume(self):
        """a pool cannot be deleted if there is a volume in it.
        """
        self._del_volume('%s/b' % self.pool_name)
        self.volume_m.create('%s/b' % self.pool_name, self.size)
        self._del_pool(self.pool_name, status_code=500)
        self._del_volume('%s/b' % self.pool_name)

        if not self.exists(self.pool_m, self.pool_name):
            self._create_pool(self.pool_name)

    def _change_path(self, path):
        idx = path.find(':')
        if idx != -1:
            path = path[idx+1:]
        return path

    @unittest.skipIf(False, 'skip me')
    def test_list(self):
        _, resp = self.pool_m.list(skip=0, limit=100)
        found = False
        if resp.records:
            self.assertEqual(resp.status_code, 200)
            for item in resp.records:
                try:
                    path = self._change_path(item['path'])
                    if path == self.pool_name:
                        found = True
                    self.pool_m.stat(path)
                except ValueError, e:
                    print e

        self.assertEqual(found, True)

    @unittest.skipIf(True, 'skip this')
    def test_nofound(self):
        self._del_volume('pool.b/a')
        self._del_pool('pool.b')

        ret, resp = self.pool_m.stat('pool.b')

        self.assertEqual(resp.status_code, 404)


class TestPoolQuota(TestBase):
    def setUp(self):
        super(TestPoolQuota, self).setUp()

        self.pname_01 = 'pool.a'
        self.vname_01 = 'pool.a/b'
        self.sname_01 = 'pool.a/b@c'
        self.clone_01 = 'pool.a/d'

        self.pool_quota = 10

        self.size = utils.GB(1)
        self.new_size = utils.GB(2)

        self._del_volume(self.vname_01)
        self._del_pool(self.pname_01, status_code=200)

        self._create_pool(self.pname_01, self.pool_quota)

        #for x in range(3):
        #    vname = '%s_%s' % (self.vname_01, x)
        #    self._create_volume(vname, self.size * 8)
        #    self._del_volume(vname)

        #_, resp = self.volume_m.resize(self.vname_01, self.size * 8)
        #self.assertEqual(resp.status_code, 200)

        #_, resp = self.volume_m.resize(self.vname_01, self.size * 9)
        #self.assertEqual(resp.status_code, 200)

    def tearDown(self):
        # self._del_volume(self.vname_01)
        self._del_pool(self.pname_01)

    @unittest.skipIf(True, 'skip me')
    def test_quota_ok(self):
        self._create_volume(self.vname_01, self.size*9, status_code=200)
        self._del_volume(self.vname_01)

    @unittest.skipIf(True, 'skip me')
    def test_quota_error(self):
        self._create_volume(self.vname_01, self.size*10, status_code=500)
        self._del_volume(self.vname_01)

    @unittest.skipIf(False, 'skip me')
    def test_stat(self):
        ret, resp = self.pool_m.stat(self.pname_01)
        self.assertEqual(resp.status_code, 200)
        #pool = resp.records
        #if pool:
        #    self.assertEqual(pool['sp_quota'], self.pool_quota)

    @unittest.skipIf(False, 'skip me')
    def test_update(self):
        quota = 100

        ret, resp = self.pool_m.update(self.pname_01, quota=quota)
        self.assertEqual(resp.status_code, 200)

        self._assertQuota(self.pname_01, quota)

    @unittest.skipIf(False, 'skip me')
    def test_clean_quota(self):
        quota = 0

        ret, resp = self.pool_m.update(self.pname_01, quota=quota)
        self.assertEqual(resp.status_code, 200)

        self._assertQuota(self.pname_01, quota)

    def _assertQuota(self, pname, quota):
        ret, resp = self.pool_m.stat(pname)
        self.assertEqual(resp.status_code, 200)
        pool = resp.records
        if pool:
            self.assertEqual(pool['sp_quota'], quota)


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    #s.addTests(load_from(TestAll))
    s.addTests(load_from(TestPoolQuota))

    return s


if __name__ == '__main__':
    unittest.main()
