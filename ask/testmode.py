#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os

# app path
CURRENT_PATH = os.path.dirname(os.path.abspath(__file__))


def is_testmode(base=CURRENT_PATH):
    fname = os.path.join(base, '__test__')
    res = os.path.isfile(fname)
    if __debug__:
        # python -O scripy.py to disable this!!!
        print '.. checking filename', fname
        if res:
            print '== Warning: Run in test mode!'
        else:
            print '== Warning: Run in normal mode!'
    return res


# app start status
TEST_MODE = is_testmode()


if TEST_MODE:
    pass
else:
    pass


# redis connection
# mongodb connection
