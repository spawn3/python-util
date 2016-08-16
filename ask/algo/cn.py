#!/usr/bin/env python
# -*- coding: utf-8 -*-


def mul(m, n):
    res = 1
    for x in range(m, n+1):
        res = res * x
    return res
	

def comp(m, n):
    return mul(m - n + 1, m) / mul(1, n)


if __name__ == '__main__':
    print comp(3, 2)
    print comp(6, 2)
    print comp(10, 2)

    for r in [1,2,3]:
        for m, n in [(72, 1), (36, 2), (24, 3), (18, 4), (9, 8)]:
            print 'N=%d, R=%d' % (m, r), n * comp(m, r)
        print
