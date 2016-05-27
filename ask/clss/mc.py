#!/usr/bin/env python
# -*- coding: utf-8 -*-


class MyMeta(type):
    def __new__(cls, name, parents, dct):
        print 'A new class named ' + name + ' is going to be created'
        return super(MyMeta, cls).__new__(cls, name, parents, dct)


class MyClass(object):
    __metaclass__ = MyMeta

