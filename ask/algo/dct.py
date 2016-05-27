#!/usr/bin/env python
# -*- coding: utf-8 -*-


from collections import defaultdict


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


def test_defaultdict():
    s = [('yellow', 1), ('blue', 2), ('yellow', 3), ('blue', 4), ('red', 1)]
    d = defaultdict(list)  # int, set
    for k, v in s:
        d[k].append(v)
    assert(len(d) == 3)


def add(*args):
    """

    >>> add(1, 2)
    4

    :param args:
    :return:
    """
    total = 0
    for x in args:
        total += x
    return total


if __name__ == '__main__':
    import doctest
    doctest.testmod()
