#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from base import LichBase, RemoteLocation
from runner import http_runner


class LichLicenseParam(RemoteLocation):

    def __init__(self, host_ip=None):
        super(LichLicenseParam, self).__init__(host_ip)


class LichLicense(LichBase):

    @http_runner()
    def sniffer(self, param):
        cmd = '%s -m sniffer' % (self.lich_license)
        return cmd


if __name__ == '__main__':
    pass
