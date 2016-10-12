#!/usr/bin/env python2
# -*- coding: utf-8 -*-


import json


def read_config(fname='gbase_config.json'):
    with open(fname) as f:
        data = json.load(f)
        print type(data), data


def init():
    conf = read_conf()
    pass


def resize():
    conf = read_conf()
    pass


def check_ready():
    conf = read_conf()
    pass


if __name__ == '__main__':
    read_config()
