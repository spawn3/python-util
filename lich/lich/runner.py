#!/usr/bin/env python2
# -*- coding: utf-8 -*-


import functools
import subprocess
import time


def timethis(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        start = time.time()
        r = func(*args, **kwargs)
        end = time.time()
        print('[{}]: {}.{} {} {}'.format(end - start, func.__module__, func.__name__, args, kwargs))
        print('\t{}'.format(r))
        print
        return r
    return wrapper


@timethis
def _exec(cmd):
    cmd = cmd.split(' ')
    try:
        out = subprocess.check_output(cmd)
        res = 0, out.splitlines()
    except subprocess.CalledProcessError as e:
        res = e.returncode, e.message.splitlines()
    except OSError as e:
        res = -1, str(e).splitlines()
    except Exception as e:
        raise e
    return res


def local_runner(exc_handler=None):
    def decorator(func):
        @functools.wraps(func)
        def wrapper(self, *args, **kw):
            res = None
            try:
                cmd = func(self, *args, **kw)
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
