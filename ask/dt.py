#!/usr/bin/env python
# -*- coding: utf-8 -*-


import datetime


def get_daystr(dt=None, day=0):
    if not dt:
        dt = datetime.datetime.now()
    dt += datetime.timedelta(days=day)
    return '%04d%02d%02d' % (dt.year, dt.month, dt.day)


def total_seconds(delta):
    return delta.days * 3600 * 24 + delta.seconds
