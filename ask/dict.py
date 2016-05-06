#!/usr/bin/env python
# -*- coding: utf-8 -*-


def dict_append(d, k, v):
    if k not in d:
        d[k] = [v]
    else:
        d[k].append(v)
    return d
