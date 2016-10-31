#!/usr/bin/env python
# -*- coding: utf-8 -*-


from .common import exception as exc


POOL = 'pool'
VOLUME = 'volume'
SNAPSHOT = 'snapshot'


class UmpPath(object):

    def __init__(self, path, protocol='iscsi', username='cinder'):
        from Ump.objs.root import init_root
        cluster = init_root().cluster

        self.protocol = protocol
        self.username = username
        self.protocol_root = cluster.protocol_root(protocol)
        self.path = path

        self._parse()

    def _reset(self):
        self.type = None
        self.pool_name = None
        self.vol_name = None
        self.snap_name = None

    def __repr__(self):
        if self.type == POOL:
            return self.pool_path
        elif self.type == VOLUME:
            return self.volume_path
        elif self.type == SNAPSHOT:
            return self.snap_path
        else:
            return 'InvalidPath: %s' % self.path

    def _parse(self, path=None):
        self._reset()

        if not path:
            path = self.path
        else:
            self.path = path
        path = path.strip()

        idx = path.find('@')
        if idx != -1:
            self.type = SNAPSHOT
            volume_name, snap_name = self._check_snapshot(path)
            pool_name, vol_name = self._check_volume(volume_name)

            self.pool_name = pool_name
            self.vol_name = vol_name
            self.snap_name = snap_name
            return

        idx = path.find('/')
        if idx != -1:
            self.type = VOLUME
            pool_name, vol_name = self._check_volume(path)
            self.pool_name = pool_name
            self.vol_name = vol_name
            return

        self.type = POOL
        self.pool_name = path

    def _check_snapshot(self, path):
        l = path.split('@')
        if len(l) != 2:
            raise exc.InvalidPath(path)
        return l[0], l[1]

    def _check_volume(self, path):
        l = path.split('/')
        if len(l) != 2:
            raise exc.InvalidPath(path)
        return l[0], l[1]

    def check_protocol(self, protocol='iscsi'):
        if protocol not in ['iscsi', 'nbd']:
            raise exc.ProtocolNotSupported(protocol)
        return True

    @property
    def long_pool_name(self):
        return '%s:%s' % (self.username, self.pool_name)

    @property
    def long_volume_name(self):
        return '%s/%s' % (self.long_pool_name, self.vol_name)

    @property
    def pool_path(self):
        return '/%s/%s' % (self.protocol_root, self.long_pool_name)

    @property
    def volume_path(self):
        return '/%s/%s' % (self.protocol_root, self.long_volume_name)

    @property
    def snap_path(self):
        return '/%s/%s@%s' % (self.protocol_root, self.long_volume_name, self.snap_name)

    def ensure(self, t):
        if t == SNAPSHOT:
            if self.type not in [SNAPSHOT]:
                raise exc.InvalidPath(self.path, t)
        elif t == VOLUME:
            if self.type not in [SNAPSHOT, VOLUME]:
                raise exc.InvalidPath(self.path, t)
        elif t == POOL:
            if self.type not in [SNAPSHOT, VOLUME, POOL]:
                raise exc.InvalidPath(self.path, t)
        else:
            raise exc.InvalidParameter(t)

    def is_pool(self):
        return self.type == POOL

    def is_volume(self):
        return self.type == VOLUME

    def is_snapshot(self):
        return self.type == SNAPSHOT


if __name__ == '__main__':
    path = UmpPath('a/b@c')
    path.ensure(POOL)
    path.ensure(VOLUME)
    path.ensure(SNAPSHOT)

    print path
    print path.long_pool_name
    print path.long_volume_name
    print path.pool_path
    print path.volume_path
    print path.snap_path
