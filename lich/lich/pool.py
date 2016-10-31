#!/usr/bin/env python2
# -*- coding: utf-8 -*-


from base import LichBase
from runner import local_runner


def _create_exc_handler(e, *args, **kw):
    if str(e).find("File exists") != -1:
        path = args[0]
        is_with_volume = kw.get('is_with_volume', False)
        if not is_with_volume :
            raise Exception("存储池%s已存在" % path)
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


class LichPool(LichBase):

    @local_runner(exc_handler=_create_exc_handler)
    def create(self, path, pdomain=None):
        if not pdomain:
            cmd = '%s mkpool %s -p %s' % (self.lichbd, path.long_pool_name, path.protocol)
        else:
            cmd = '%s mkpool %s -p %s -A %s' % (self.lichbd, path.long_pool_name, path.protocol, pdomain)
        return cmd

    @local_runner(exc_handler=_delete_exc_handler)
    def delete(self, path):
        cmd = '%s rmpool %s -p %s' % (self.lichbd, path.long_pool_name, path.protocol)
        return cmd

    @local_runner()
    def list(self, path):
        cmd = '%s lspools -p %s' % (self.lichbd, path.protocol)
        return cmd

    def stat(self, path):
        retcode, lines = self.list(path)
        if retcode != 0:
            return False
        for line in lines:
            l = line.split(' ')
            if l[len(l)-1] == path.long_pool_name:
                return True
        return False

    def exists(self, path):
        return self.stat(path)


class LichCreatePool(LichBase):
    def __init__(self, path):
        super(LichCreatePool, self).__init__()
        self.path = path

    def do(self):
        pass

    def undo(self):
        pass


if __name__ == '__main__':
    from umptypes import UmpPath

    path = UmpPath('pool1')
    vol = LichPool()
    vol.create(path)
    print vol.list(path)
    vol.delete(path)
    print vol.list(path)
