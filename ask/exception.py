#!/usr/bin/env python
# -*- coding: utf-8 -*-


class MyException(Exception):
    def __init__(self, errno, msg='MyException'):
        self.args = (errno, msg)
        self.errno = errno
        self.msg = msg


class DataException(MyException):
    pass
