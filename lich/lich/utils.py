#!/usr/bin/env python2
# -*- coding: utf-8 -*-


import re


def split_lines(s):
    l = s.split('\n')
    res = [x.strip() for x in l if x.strip()]
    return res


def parse_split(s, sep_char=' '):
    l = s.split(sep_char)
    res = [ x.strip() for x in l if x.strip()]
    return res


def parse_int(x):
    try:
        x = x.strip()
        res = int(x)
    except Exception, e:
        res = x
    return res


def parse_kv(s, sep_char=':', replace_char=None, skip_char='#', val_list=False):
    l = split_lines(s)
    res = {}
    for kv in l:
        kv = parse_split(kv, sep_char=sep_char)
        if len(kv) != 2:
            continue
        k = kv[0].strip()
        if not k:
            continue
        if k.startswith(skip_char):
            continue
        if replace_char:
            k = k.replace(' ', replace_char)
        v = parse_int(kv[1])
        if val_list:
            v = v.strip().split(',')
        res[k] = v
    return res


def regexp_str(s, expression, sep=' '): 
    mlist = re.compile(expression).findall(s)
    return sep.join(mlist)


def parse_etc_hosts(s):
    return parse_kv(s, sep_char=' ', replace_char=None)


def parse_health(s, sep_char=':'):
    return parse_kv(s, sep_char=':', replace_char='_')


def parse_global_config(s, sep_char=':'):
    return parse_kv(s, sep_char=sep_char, replace_char=None)


def parse_listnode(s, sep_char=':'):
    s = regexp_str(s, r'(?:INFO:|ERROR:|WARNING:|WARN:|DEBUG:)(.*)', sep='\n')
    return parse_kv(s, sep_char=sep_char, replace_char=None)


def parse_connection(s, sep_char=':'):
    return parse_kv(s, sep_char=sep_char, val_list=True)
