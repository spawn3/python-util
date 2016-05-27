#!/usr/bin/env python
# -*- coding: utf-8 -*-

import types
import math
from pprint import pprint

# All Python objects have the folling three characteristics: an identity, a type and a value.


def test_1():
    o = object()
    pprint(dir(o))
    # [
    # '__doc__',
    # '__class__',
    # '__new__', '__init__',
    # '__format__', '__str__', '__repr__',
    # '__hash__',
    # '__setattr__', '__getattribute__', '__delattr__',
    # '__sizeof__',
    # '__reduce__', '__reduce_ex__',
    # '__subclasshook__',
    # ]


class A(object):
    #def __new__(cls, *args, **kwargs):
    #    print args
    #    print kwargs

    def __init__(self, radius):
        self.radius = radius

    @property
    def area(self):
        return math.pi * self.radius ** 2

    @property
    def perimeter(self):
        return 2 * math.pi * self.radius

    @staticmethod
    def sm(*args, **kwargs):
        print args
        print kwargs

    @classmethod
    def cm(cls, *args, **kwargs):
        print cls
        print args
        print kwargs


class B(A):
    def __init__(self, *args, **kwargs):
        super(B, self).__init__(*args, **kwargs)


class C(object):
    # __slots__ = ['ice', 'cream']
    number = 0

    def __init__(self, number):
        # super(C, self).__init__()
        C.number = number


def test_metaclass():
    class Foo(object):
        bar = True

    type('Foo', (), {'bar': True})


# pprint(globals())

if __name__ == '__main__':
    a = A(1)
    print a.area

