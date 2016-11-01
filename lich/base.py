#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest


from lich.umptypes import UmpPath
from lich.pool import LichPool
from lich.volume import LichVolume
from lich.snapshot import LichSnapshot


RET_OK = 0


class TestBase(unittest.TestCase):
    def setUp(self):
        self.lich_pool = LichPool()
        self.lich_volume = LichVolume()
        self.lich_snapshot = LichSnapshot()

    def tearDown(self):
        pass

    def find(self, manager, name):
        ret, resp = manager.list(skip=0, limit=100)
        self.assertEqual(resp.status_code, RET_OK)
        if resp.records:
            for item in resp.records:
                if name == item['name']:
                    return item
        return None

    def find_and_delete(self, manager, name):
        item = self.find(manager, name)
        if item:
            self._delete(manager, item['id'])

    def _test_list(self, manager, find_id=None, fields=[]):
        ret, resp = manager.list(skip=0, limit=100)
        self.assertEqual(ret, RET_OK)
        found = False
        if resp.records:
            for item in resp.records:
                try:
                    for field in fields:
                        self.assertIn(field, item)

                    _id = item['id']
                    if find_id:
                        if _id == find_id:
                            found = True
                    manager.stat(_id)
                except ValueError, e:
                    print e

        if find_id:
            self.assertEqual(found, True)

    # utilities
    def _delete(self, manager, path, status_code=RET_OK):
        exists = self.exists(manager, path)
        if exists:
            ret, resp = manager.delete(path)
            self.assertEqual(ret, status_code)
            return ret, resp

    def stat(self, manager, path, status_code=RET_OK):
        ret, resp = manager.stat(path)
        self.assertEqual(ret, status_code)
        return ret, resp

    def exists(self, manager, path):
        return manager.exists(path)

    # POOL CRUD
    def create_pool(self, path, pool_quota=None, status_code=RET_OK):
        ret, resp = self.lich_pool.create(path)
        self.assertEqual(ret, status_code)
        return ret, resp

    def del_pool(self, path, status_code=RET_OK):
        return self._delete(self.lich_pool, path, status_code=status_code)

    def stat_pool(self, path, status_code=RET_OK):
        return self.stat(self.lich_pool, path, status_code)

    def list_pools(self):
        path = UmpPath('a/b@c')
        rc, pools = self.lich_pool.list(path)
        return pools

    # VOLUME CRUD
    def create_volume(self, path, size, status_code=RET_OK):
        ret, resp = self.lich_volume.create(path, size)
        self.assertEqual(ret, status_code)
        return ret, resp

    def del_volume(self, path, status_code=RET_OK):
        return self._delete(self.lich_volume, path, status_code=status_code)

    def stat_volume(self, path, status_code=RET_OK):
        return self.stat(self.lich_volume, path, status_code)

    # SNAPSHOT CRUD
    def create_snapshot(self, path, status_code=RET_OK):
        ret, resp = self.lich_snapshot.create(path)
        self.assertEqual(ret, status_code)
        return ret, resp

    def del_snapshot(self, path, status_code=RET_OK):
        return self._delete(self.lich_snapshot, path, status_code)

    def stat_snapshot(self, path, status_code=RET_OK):
        return self.stat(self.lich_snapshot, path, status_code)
