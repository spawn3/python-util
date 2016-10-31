#!/usr/bin/env python2
# -*- coding: utf-8 -*-


import functools
import traceback


def _http_runner(func):
    @functools.wraps(func)
    def wrapper(self, *args, **kw):
        cmd = func(self, *args, **kw)
        try:
            param = args[0]
            res = self.api_sync_call(param.host_ip, cmd)
        except Exception, e:
            # TODO
            print e
            raise e
        return res
    return wrapper


def http_runner(exc_handler=None):
    def decorator(func):
        @functools.wraps(func)
        def wrapper(self, *args, **kw):
            res = None
            try:
                cmd = func(self, *args, **kw)
                param = args[0]
                res = self.api_sync_call(param.host_ip, cmd)
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


def ssh_runner(exc_handler=None):
    def decorator(func):
        @functools.wraps(func)
        def wrapper(self, *args, **kw):
            res = None
            try:
                cmd = func(self, *args, **kw)
                param = args[0]
                res = self._execute_remote(param.host_ip, cmd, username=param.username, password=param.password)
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
