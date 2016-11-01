#!/usr/bin/env python2
# -*- coding: utf-8 -*-


def GB(n):
    return n * 1024 * 1024 * 1024


def timeit():
    pass


def list_is_equal(l1, l2):
    if len(l1) != len(l2):
        return False
    for x in l1:
        if x not in l2:
            return False
    return True
