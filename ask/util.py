#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from pprint import pprint

reload(sys)
sys.setdefaultencoding('utf-8')

import pdb
import profile
import pyclbr

import importlib

# relative import
from . import mc
import obj


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
