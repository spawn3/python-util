#!/usr/bin/env python
# -*- coding: utf-8 -*-


import json


DEFS = {
        'A': 'A desc',
        'B': 'B desc',
        }


class A(Exception):
    def __init__(self, **kw):
        name = self.__class__.__name__
        print 'Enter %s.__init__' % name
        name = DEFS.get(name, '')
        if kw:
            name = '%s: %s' % (name, json.dumps(kw))
        super(A, self).__init__(name)

class B(A):
    pass


if __name__ == '__main__':
    raise A(m=1, n=2)
    raise B(m=1, n=2)
