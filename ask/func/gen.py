#!/usr/bin/env python
# -*- coding: utf-8 -*-


import math
import types
import itertools
from contextlib import contextmanager


def fibonacci():
    a, b = 0, 1
    while True:
        yield b
        a, b = b, a + b


def f(*args, **kwargs):
    print args
    print kwargs


def gensquares(N):
    for i in range(N):
        yield i ** 2


def countdown(n):
    print 'Couting down!'
    while n > 0:
        yield n
        n -= 1


def coroutine(func):
    def start(*args, **kwargs):
        g = func(*args, **kwargs)
        g.next()
        return g
    return start


@coroutine
def receiver():
    print 'Ready to receive'
    while True:
        n = (yield)
        print 'Got %s' % n


@coroutine
def line_splitter(delimiter=None):
    print 'Ready to split'
    result = None
    while True:
        line = (yield result)
        result = line.split(delimiter)


def is_prime(number):
    if number > 1:
        if number == 2:
            return True
        if number % 2 == 0:
            return False

        for current in range(3, int(math.sqrt(number) + 1), 2):
            if number % current == 0:
                return False
        return True
    return False


def get_primes(input_list):
    return (x for x in input_list if is_prime(x))


def test():
    print 'step 1...'
    res = yield 10
    print 'step 2...', res


def test1():
    for i in range(3):
        m = yield i
        print m


if __name__ == '__main__':
    gen = test1()
    print gen.next()
    print gen.next()
    print gen.next()
    print gen.next()
