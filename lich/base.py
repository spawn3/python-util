#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest


from fusionstor import (
    TokenManager,
    UserManager,
    PoolManager,
    VolumeManager,
    SnapshotManager,
    QosManager,
)


class TestBase(unittest.TestCase):
    def setUp(self):
        self.token_m = TokenManager()
        self.user_m = UserManager()
        self.pool_m = PoolManager()
        self.volume_m = VolumeManager()
        self.snapshot_m = SnapshotManager()
        self.qos_m = QosManager()

        #user = self.find(self.token_m, 'user001')
        #if user:
        #    _, resp = self.token_m.delete(user['id'])
        #    self.assertEqual(resp.status_code, 200)

        ret, resp = self.token_m.create()
        self.assertEqual(resp.status_code, 200)

    def tearDown(self):
        pass

    def exists(self, manager, id_or_path):
        return manager.exists(id_or_path)

    def find(self, manager, name):
        ret, resp = manager.list(skip=0, limit=100)
        self.assertEqual(resp.status_code, 200)
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
        self.assertEqual(resp.status_code, 200)
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

    def _create_pool(self, pname, pool_quota=None, status_code=200):
        ret, resp = self.pool_m.create(pname, quota=pool_quota)
        self.assertEqual(resp.status_code, status_code)
        return ret, resp

    def _create_volume(self, vname, size, status_code=200):
        ret, resp = self.volume_m.create(vname, size)
        self.assertEqual(resp.status_code, status_code)
        return ret, resp

    def _create_snapshot(self, snap_name, status_code=200):
        ret, resp = self.snapshot_m.create(snap_name)
        self.assertEqual(resp.status_code, status_code)
        return ret, resp

    def _del_pool(self, pname, status_code=200):
        return self._delete(self.pool_m, pname, status_code=status_code)

    def _del_volume(self, vname, status_code=200):
        return self._delete(self.volume_m, vname, status_code=status_code)

    def _del_snapshot(self, snap_name, status_code=200):
        return self._delete(self.snapshot_m, snap_name, status_code)

    def stat_pool(self, id_or_path, status_code=200):
        return self._stat(self.pool_m, id_or_path, status_code)

    def stat_volume(self, id_or_path, status_code=200):
        return self._stat(self.volume_m, id_or_path, status_code)

    def stat_snapshot(self, id_or_path, status_code=200):
        return self._stat(self.snapshot_m, id_or_path, status_code)

    def _delete(self, manager, id_or_path, status_code=200):
        if self.exists(manager, id_or_path):
            ret, resp = manager.delete(id_or_path)
            self.assertEqual(resp.status_code, status_code)
            return ret, resp

    def _stat(self, manager, id_or_path, status_code=200):
        ret, resp = manager.stat(id_or_path)
        self.assertEqual(resp.status_code, status_code)
        return ret, resp
