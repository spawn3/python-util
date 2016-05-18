#!/usr/bin/env python
# -*- coding: utf-8 -*-


import functools


enable_tracing = True
if enable_tracing:
    debug_log = open('debug.log', 'w')


def trace(func):
    if enable_tracing:
        def callf(*args, **kwargs):
            debug_log.write('Calling %s: %s, %s\n' % (func.__name__, args, kwargs))
            r = func(*args, **kwargs)
            debug_log.write('%s returned %s\n' % (func.__name__, r))
            return r
        return callf
    else:
        return func


@trace
def square(x):
    return x * x


def wrap(func):
    @functools.wraps(func)
    def call(*args, **kwargs):
        return func(*args, **kwargs)
    return call


if __name__ == '__main__':
    square(1)

