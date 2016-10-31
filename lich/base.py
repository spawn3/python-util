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
        _, resp = manager.list(skip=0, limit=100)
        self.assertEqual(resp.status_code, RET_OK)
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

    def stat(self, manager, id_or_path, status_code=RET_OK):
        ret, resp = manager.stat(id_or_path)
        self.assertEqual(resp.status_code, status_code)
        return ret, resp

    def exists(self, manager, id_or_path):
        return manager.exists(id_or_path)

    def _delete(self, manager, id_or_path, status_code=RET_OK):
        if self.exists(manager, id_or_path):
            ret, resp = manager.delete(id_or_path)
            self.assertEqual(resp.status_code, status_code)
            return ret, resp

    def _create_pool(self, pname, pool_quota=None, status_code=RET_OK):
        path = UmpPath(pname)
        retcode, resp = self.lich_pool.create(path)
        self.assertEqual(retcode, status_code)
        return retcode, resp

    def _del_pool(self, pname, status_code=RET_OK):
        path = UmpPath(pname)
        return self._delete(self.lich_pool, path, status_code=status_code)

    def stat_pool(self, id_or_path, status_code=RET_OK):
        return self.stat(self.lich_pool, id_or_path, status_code)

    def _create_volume(self, vname, size, status_code=RET_OK):
        ret, resp = self.lich_volume.create(vname, size)
        self.assertEqual(resp.status_code, status_code)
        return ret, resp

    def _del_volume(self, vname, status_code=RET_OK):
        return self._delete(self.lich_volume, vname, status_code=status_code)

    def stat_volume(self, id_or_path, status_code=RET_OK):
        return self.stat(self.lich_volume, id_or_path, status_code)

    def _create_snapshot(self, snap_name, status_code=RET_OK):
        ret, resp = self.lich_snapshot.create(snap_name)
        self.assertEqual(resp.status_code, status_code)
        return ret, resp

    def _del_snapshot(self, snap_name, status_code=RET_OK):
        return self._delete(self.lich_snapshot, snap_name, status_code)

    def stat_snapshot(self, id_or_path, status_code=RET_OK):
        return self.stat(self.lich_snapshot, id_or_path, status_code)
