#!/usr/bin/env python
# -*- coding: utf-8 -*-


def dict_append(d, k, v):
    if k not in d:
        d[k] = []
    d[k].append(v)
    return d


def dict_inc(d, k, v):
    if k not in d:
        d[k] = 0
    d[k] += v
    return d

dict()
