#!/usr/bin/env python2
# -*- coding: utf-8 -*-


from Ump.common.utils import inspect_func

from base import RemoteLocation, LichBase
from runner import ssh_runner, http_runner
import utils


class LichShellParam(RemoteLocation):
    def __init__(self, host_ip=None, cmd='echo OK', password='mdsmds', is_http=False):
        super(LichShellParam, self).__init__(host_ip, password=password)

        self.is_http = is_http
        self.cmd = cmd


class LichShell(LichBase):

    @ssh_runner()
    def ssh_run(self, param):
        return param.cmd

    @http_runner()
    def http_run(self, param):
        return param.cmd

    def run(self, param):
        if param.is_http:
            return self.http_run(param)
        else:
            return self.ssh_run(param)

    def exists_base(self, param, name, _type):
        FLAG = 'noprogram'
        param.cmd = "if [ ! -%s %s ]; then echo '%s'; fi" % (_type, name, FLAG)
        res = self.run(param)
        l = utils.split_lines(res)
        return len(l) != 1 or l[0] != FLAG

    @inspect_func
    def dir_exists(self, param, dname):
        return self.exists_base(param, dname, 'd')

    @inspect_func
    def file_exists(self, param, fname):
        return self.exists_base(param, fname, 'f')

    @inspect_func
    def rm(self, param, fname):
        param.cmd = "if [ -f %s ]; then rm -rf %s; fi" % (fname, fname)
        res = self.run(param)
        return res

    def cat_file(self, param, fname):
        param.cmd = "cat %s" % (fname)
        res = self.run(param)
        return res

    @inspect_func
    def dump_config(self, param):
        param.cmd = '%s configdump' % (self.lich)
        res = self.run(param)
        res = utils.parse_global_config(res)
        return res

    @inspect_func
    def read_etc_hosts(self, param, host_ip='0.0.0.0'):
        if not host_ip:
            host_ip = param.host_ip
        param.cmd = '%s etc_hosts --host %s' % (self.lich_syncump, host_ip)
        res = self.run(param)
        res = utils.parse_etc_hosts(res)
        return res

    def node_start(self, param):
        param.cmd = '%s --start' % self.lich_node
        res = self.run(param)
        return res

    def node_stop(self, param):
        param.cmd = '%s --stop' % self.lich_node
        res = self.run(param)
        return res

    @inspect_func
    def node_list(self, param):
        param.cmd = '%s listnode' % self.lich
        res = self.run(param)
        return res
    
    @inspect_func
    def sethosts(self, param, host, hostname):
        param.cmd = "%s --sethosts %s %s" % (self.lich_node, host, hostname)
        res = self.run(param)
        return res

    @inspect_func
    def cluster_create(self, param, hosts):
        param.cmd = "%s create %s" % (self.lich, hosts)
        res = self.run(param)
        return res

    @inspect_func
    def addnode(self, param, hosts):
        param.cmd = "%s addnode %s" % (self.lich, hosts)
        res = self.run(param)
        return res

    @inspect_func
    def sshkey(self, param, sshkey_hosts=None, password=None):
        '''
            sshkey_hosts:  create the ssh keys; type list or string  
        '''
        sshkey_hosts = " ".join(sshkey_hosts) if isinstance(sshkey_hosts, list) else sshkey_hosts
        param.cmd = '''
                        expect -c 'set timeout 2;
                        spawn %s sshkey %s;
                        expect "input password:" {send "%s\r"};
                        interact'
                    ''' % (self.lich, sshkey_hosts, password)
        res = self.run(param)
        return True


if __name__ == '__main__':
    param = LichShellParam(host_ip='192.168.120.71', password='mdsmds')
    obj = LichShell()
    obj.file_exists(param, 'lich/admin/cluster.py')
    obj.dump_config(param)
    obj.read_etc_hosts(param)
    obj.node_list(param)
