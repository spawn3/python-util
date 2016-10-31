#!/usr/bin/env python2
# -*- coding: utf-8 -*-


from base import LichBase
from runner import local_runner


def _create_exc_handler(e, *args, **kw):
    if str(e).find("File exists") != -1:
        param = args[0]
        is_with_volume = kw.get('is_with_volume', False)
        if not is_with_volume :
            raise Exception("存储池%s已存在" % param.path)
    else:
        raise Exception(e)


def _delete_exc_handler(e, *args, **kw):
    if str(e).find('No such file or directory') != -1:
        pass
    else:
        raise Exception(e)


def _remove_attr_exc_handler(e, *args, **kw):
    ignoreNoKey = kw.get('ignoreNoKey')
    if ignoreNoKey and "Required key not available" in str(e):
        pass
    else:
        raise Exception(e)


class LichPoolParam(object):
    def __init__(self, path, pro_domain=None):

        self.path = path
        self.pro_domain = pro_domain


class LichPool(LichBase):

    @local_runner(exc_handler=_create_exc_handler)
    def create(self, param):
        path = param.path
        if not param.pro_domain:
            cmd = '%s mkpool %s -p %s' % (self.lichbd, path.long_pool_name, path.protocol)
        else:
            cmd = '%s mkpool %s -p %s -A %s' % (self.lichbd, path.long_pool_name, path.protocol, param.pro_domain)
        return cmd

    @local_runner(exc_handler=_delete_exc_handler)
    def delete(self, param):
        path = param.path
        cmd = '%s rmpool %s -p %s' % (self.lichbd, path.long_pool_name, path.protocol)
        return cmd

    @local_runner()
    def list(self, param):
        path = param.path
        cmd = '%s lspools -p %s' % (self.lichbd, path.protocol)
        return cmd

    def exists(self, param):
        raise NotImplementedError


class LichCreatePool(LichBase):
    def __init__(self, param):
        super(LichCreatePool, self).__init__()
        self.param = param

    def do(self):
        pass

    def undo(self):
        pass


if __name__ == '__main__':
    from umptypes import UmpPath

    path = UmpPath('pool1')
    param = LichPoolParam(path)
    vol = LichPool()
    vol.create(param)
    print vol.list(param)
    vol.delete(param)
    print vol.list(param)
