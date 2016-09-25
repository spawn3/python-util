#!/usr/bin/env python
# -*- coding: utf-8 -*-

import datetime
import sys
from json import JSONEncoder

from repoze.lru import lru_cache

reload(sys)
sys.setdefaultencoding('utf-8')


# relative import


def parts(l, n):
    n = max(1, n)
    return [l[i:i+n] for i in range(0, len(l), n)]


def fibonacci():
    a, b = 0, 1
    while True:
        yield b
        a, b = b, a+b


class A(object):
    pass


# pprint(globals())

globals()['z'] = 7
print 'z =', z


class MongoEncoder(JSONEncoder):
    def default(self, obj, **kwargs):
        if isinstance(obj, datetime.datetime):
            return obj.isoformat()
        else:
            return JSONEncoder.default(obj, **kwargs)
