#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest


from fusionstor import (
        ClusterManager, 
        PDomainManager,
        HostManager, 
        UserManager,
        PoolManager, 
        VolumeManager, 
        SnapshotManager,
        )

from base import TestBase


class TestList(TestBase):
    def setUp(self):
        super(TestList, self).setUp()

        self.managers = [ClusterManager, PDomainManager, HostManager, UserManager, PoolManager, VolumeManager,
                         SnapshotManager]

    def test_list(self):
        for cls in self.managers:
            m = cls()
            ret, resp = m.list(skip=0, limit=100)
            if resp.records:
                self.assertEqual(resp.status_code, 200)
                for item in resp.records:
                    try:
                        if cls in [PoolManager, VolumeManager, SnapshotManager]:
                            path = item['path']
                            idx = path.find(':')
                            if idx != -1:
                                path = path[idx+1:]
                            m.stat(path)
                        else:
                            _id = item['id']
                            m.stat(_id)
                    except ValueError, e:
                        print e


def suite():
    s = unittest.TestSuite()
    load_from = unittest.defaultTestLoader.loadTestsFromTestCase

    for tc in [TestList]:
        s.addTests(load_from(tc))

    return s


if __name__ == '__main__':
    unittest.main()
