#!/usr/bin/env python2
# -*- coding: utf-8 -*-


import functools
import traceback

import subprocess


def _exec(cmd):
    if isinstance(cmd, list):
        cmd = ' '.join(cmd)
    print '--- cmd', cmd
    return subprocess.call(cmd, shell=True)


def local_runner(exc_handler=None):
    def decorator(func):
        @functools.wraps(func)
        def wrapper(self, *args, **kw):
            res = None
            try:
                cmd = func(self, *args, **kw)
                param = args[0]
                res = _exec(cmd)
            except Exception, e:
                # TODO
                # traceback.print_exc()
                if exc_handler:
                    exc_handler(e, *args, **kw)
                else:
                    raise
            return res
        return wrapper
    return decorator
