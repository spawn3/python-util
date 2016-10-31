#!/usr/bin/env python2
# -*- coding: utf-8 -*-


from pyvalid import accepts, returns


@accepts(int, int)
def calc(x, y):
    return x+y


@returns(bool)
def rb():
    return 1



if __name__ == '__main__':
    calc(1, 1.0)
    rb()
