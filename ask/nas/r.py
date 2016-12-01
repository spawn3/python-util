#!/usr/bin/env python
# -*- coding: utf-8 -*-


import sys
import datetime
import time
from redis import Redis
import json
import traceback
from optparse import OptionParser

from repoze.lru import lru_cache



NODEID_ROOT = '1'
META = '@'


DEBUG = True


def print2(*args):
    if DEBUG:
        print ' '.join([str(x) for x in args])


class Fs(object):

    def __init__(self):
        self.r = Redis()
        self.counter = 0
        self.version = int(time.time())

    def _newid(self):
        self.counter += 1
        return '%d_%s' % (self.counter, self.version)

    def _split_path(self, path):
        if path.startswith('/'):
            return path.split('/')[1:]
        else:
            return path.split('/')

    def _meta(self, node_id):
        meta = self.r.hget(node_id, META)
        return json.loads(meta)

    def _get_nodeid(self, parent_id=None, name='/'):
        if parent_id is None:
            return NODEID_ROOT

        node = self.r.hget(parent_id, name)
        return node

    def _exists(self, parent_id, name):
        node = self._get_nodeid(parent_id, name)
        if node:
            return True
        return False

    @lru_cache(maxsize=10000, timeout=600)
    def _path2pair(self, path):
        """ TODO
        - rename
        - hardlink/symlink

        :param path:
        :return:
        """
        l = self._split_path(path)
        parent_id = NODEID_ROOT
        node_id = None
        res = (parent_id, node_id)
        for x in l:
            # if parent_id is None:
            #     break
            node_id = self._get_nodeid(parent_id, x)
            print2('-- find %s/%s %s' % (parent_id, node_id, x))
            res = (parent_id, node_id)
            if node_id:
                parent_id = node_id
            else:
                break

        return res

    def _newnode(self, parent_id, node_id, name, meta):
        print2('-- write node info', node_id, name, meta)
        self.r.hset(node_id, META, json.dumps(meta))
        print2('-- register node info to parent: %s/%s %s' % (parent_id, node_id, name))
        self.r.hset(parent_id, name, node_id)
        print2()

    def mkdir(self, path):
        parent_id, node_id = self._path2pair(path)
        if node_id:
            raise

        #
        l = self._split_path(path)
        node_id = self._newid()
        name = l[-1]

        meta = {'n': name, 't': 'd'}
        self._newnode(parent_id, node_id, name, meta)

    def ls(self, path):
        parent_id, node_id = self._path2pair(path)
        if not node_id:
            raise


    def touch(self, path):
        parent_id, node_id = self._path2pair(path)
        if node_id:
            raise

        meta = self._meta(parent_id)
        if meta['t'] != 'd':
            raise

        #
        l = self._split_path(path)
        node_id = self._newid()
        name = l[-1]

        meta = {'n': name, 't': 'f'}
        self._newnode(parent_id, node_id, name, meta)

    def _scan(self, parent_id, depth=1):
        if depth == 1:
            print2(parent_id, '@', self.r.hget(parent_id, META))

        count = 0
        for k, v in self.r.hscan_iter(parent_id):
            print2('\t' * depth, k, v)
            if k != META:
                self._scan(v, depth=depth+1)
                count += 1

    def scan(self):
        parent_id = NODEID_ROOT
        self._scan(parent_id)

    def scan2(self):
        for k in self.r.scan_iter():
            self._scan(k)


TOTAL = 10


def test_xxx():
    r = Redis()
    for i in range(TOTAL):
        r.set('%d' % i, 'abcd'*25)
        if i % 100000 == 0:
            print datetime.datetime.now(), '%d/%d' % (i, TOTAL)


def test_mkdir():
    fs = Fs()
    try:
        fs.mkdir('a')
    except:
        # traceback.print_exc()
        pass

    for i in range(10):
        try:
            fs.touch('a/%s' % i)
        except:
            # traceback.print_exc()
            pass
        if i % 100000 == 0:
            print datetime.datetime.now(), '%d/%d' % (i, TOTAL)


def test_scan():
    fs = Fs()
    fs.scan2()


if __name__ == '__main__':
    parser = OptionParser()
    # parser.add_option("-f", "--file", dest="filename", help="write report to FILE", metavar="FILE")
    # parser.add_option("-q", "--quiet", action="store_false", dest="verbose", default=True, help="don't print status messages to stdout")
    parser.add_option("-d", "--dir", action="store_true", dest="dir", default=False, help="test dir")
    parser.add_option("-s", "--scan", action="store_true", dest="scan", default=False, help="test scan")
    parser.add_option("-f", "--find", action="store_true", dest="find", default=False, help="test find")
    (options, args) = parser.parse_args()

    print options, args

    if options.dir:
        test_mkdir()

    if options.scan:
        test_scan()

    if options.find:
        fs = Fs()
        fs._path2pair('/a')
