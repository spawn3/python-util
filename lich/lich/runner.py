#!/usr/bin/env python2
# -*- coding: utf-8 -*-


import functools
import subprocess


def _exec(cmd):
    # if isinstance(cmd, list):
    #     cmd = ' '.join(cmd)
    cmd = cmd.split(' ')
    print '--- cmd', cmd
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
