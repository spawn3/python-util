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



ROOT_NODEID = '1'
ROOT_PATH = '/'

META = '@'


DEBUG = True


def print2(*args):
    if DEBUG:
        print ' '.join([str(x) for x in args])


class Meta(object):

    def chown(self):
        pass

    def chmod(self):
        pass

    def repnum(self):
        pass


class Fs(object):

    def __init__(self):
        self.r = Redis()
        self.counter = 0
        self.version = int(time.time())

        #
        meta = {'t': 'd', 'n': ROOT_PATH}
        self.r.hset(ROOT_NODEID, META, meta)

    def _newid(self):
        self.counter += 1
        return '%d_%s' % (self.counter, self.version)

    def _split_path(self, path):
        l = path.split('/')
        if path[0] == ROOT_PATH:
            l[0] = '/'
        else:
            l.insert(0, '/')
        return l

    def _meta(self, node_id):
        meta = self.r.hget(node_id, META)
        if meta:
            return json.loads(meta)
        else:
            return meta

    def _get_nodeid(self, parent_id=None, name='/'):
        if parent_id is None or name == '/':
            return ROOT_NODEID

        node = self.r.hget(parent_id, name)
        return node

    def _exists(self, parent_id, name):
        node = self._get_nodeid(parent_id, name)
        if node:
            return True
        return False

    # @lru_cache(maxsize=10000, timeout=600)
    def _path2pair(self, path):
        """ TODO
        - rename
        - hardlink/symlink

        :param path:
        :return:
        """
        res = None
        l = self._split_path(path)
        parent_id = None
        for i, x in enumerate(l):
            # first is /
            node_id = self._get_nodeid(parent_id, x)
            print2('-- find %s/%s %s' % (parent_id, x, node_id))
            res = (parent_id, node_id)
            if node_id:
                parent_id = node_id
            else:
                break

        return res

    def _newnode(self, parent_id, node_id, name, meta):
        # hmset
        print2('-- hset', node_id, META, meta)
        self.r.hset(node_id, META, json.dumps(meta))
        print2('-- hset: %s/%s %s' % (parent_id, name, node_id))
        self.r.hset(parent_id, name, node_id)

    def _delnode(self, parent_id, node_id, name, meta):
        pass

    def _scan(self, node_id, depth=1):
        if depth == 1:
            print2(node_id, '@', self.r.hget(node_id, META))

        count = 0
        for k, v in self.r.hscan_iter(node_id):
            if k != META:
                print2('\t' * depth, k, v)
                self._scan(v, depth=depth+1)
                count += 1

    def scan(self):
        parent_id = ROOT_NODEID
        self._scan(parent_id)

    def scan2(self):
        for k in self.r.scan_iter():
            self._scan(k)

    def mkdir(self, path):
        print2('mkdir', path)
        parent_id, node_id = self._path2pair(path)
        if node_id:
            raise

        #
        l = self._split_path(path)
        node_id = self._newid()
        name = l[-1]

        meta = {'n': name, 't': 'd'}
        self._newnode(parent_id, node_id, name, meta)

    def _delete(self, path, is_dir=False):
        parent_id, node_id = self._path2pair(path)
        if not node_id:
            raise

        meta = self._meta(node_id)
        typ = 'd' if is_dir else 'f'
        if meta['t'] != typ:
            raise

        # empty
        if not self.is_empty(node_id):
            raise

        print2('-- hdel', parent_id, meta['n'])
        self.r.hdel(parent_id, meta['n'])
        print2('-- delete', node_id)
        self.r.delete(node_id)

    def rmdir(self, path):
        print2('rmdir', path)
        return self._delete(path, is_dir=True)

    def nchildren(self, node_id):
        return self.r.hlen(node_id) - 1

    def is_empty(self, node_id):
        return self.nchildren(node_id) == 0

    def ls(self, path):
        parent_id, node_id = self._path2pair(path)
        if not node_id:
            raise

        # TODO
        for k, v in self.r.hscan_iter(node_id):
            print k, v

    def touch(self, path):
        print2('touch', path)
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

    def unlink(self, path):
        print2('unlink', path)
        return self._delete(path, is_dir=False)

    def write(self, buf, n):
        pass

    def read(self, n):
        pass


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
        fs.ls('a')
        fs.touch('a/0')
        fs.unlink('a/0')
        fs.rmdir('a')
    except:
        # traceback.print_exc()
        pass

    return

    for i in range(10):
        try:
            fs.touch('a/%s' % i)
        except:
            # traceback.print_exc()
            pass
        finally:
            print2()
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
