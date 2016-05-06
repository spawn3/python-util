#!/usr/bin/env python
# -*- coding: utf-8 -*-


def gensquares(N):
    for i in range(N):
        yield i ** 2


def f(*args, **kwargs):
    print args
    print kwargs
