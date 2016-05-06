#!/usr/bin/env python
# -*- coding: utf-8 -*-


def parts(l, n):
    n = max(1, n)
    return [l[i:i+n] for i in range(0, len(l), n)]
