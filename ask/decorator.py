#!/usr/bin/env python
# -*- coding: utf-8 -*-

import functools
from pprint import pprint


def decorator(func):
    _state = {}

    @functools.wraps(func)
    def wrapper(*args, **kw):
        print 'input', func, args, kw
        res = func(*args, **kw)
        print 'after calling', func
        print func.__name__
        #print func.a
        pprint(_state)
        #pprint(dir(func))
        #pprint(func.__dict__)
        #pprint(func.__name__)
        print 'output', res
        return res
    #set_f_attr(wrapper, 'a', 2)
    return wrapper

def set_f_attr(f, k, v):
    print f, 'set %s.%s to %s' % (f.__name__, k, v)
    setattr(f, k, v)


@decorator
def f(*args, **kw):
    #_state['a'] = 1
    pprint(dir(f))
    set_f_attr(f, 'a', 2)
    return 1


def g():
    g.a = 1
    #setattr(g, 'a', 1)
    return 1


def h(func):
    def wrapper(*args, **kw):
        res = func(*args, **kw)
        print func.a
        return res
    return wrapper

#g.a = 2

#pprint(g.__dict__)


if __name__ == '__main__':
    #f = decorator(f)
    f()
    print f
    #pprint(dir(f))
    #pprint(dir(g))

    #print f.a

    #h(g)()
