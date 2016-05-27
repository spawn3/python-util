#!/usr/bin/env python

import functools
import sys


__all__ = ['register']


def register(dct={}):
    # @functools.wraps(cls)
    def wrapper(cls):
        print 'registering %s: %s' % (cls.__name__, cls)
        dct[cls.__name__] = cls
        dct['the%s' % cls.__name__] = cls()
        return cls
    return wrapper


def register2(name=__name__):
    m = sys.modules[name]

    def wrapper(cls):
        print 'registering %s: %s' % (cls.__name__, cls)
        setattr(m, cls.__name__, cls)
        setattr(m, 'the%s' % cls.__name__, cls())
        return cls
    return wrapper
