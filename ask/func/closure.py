#!/usr/bin/env python
# -*- coding: utf-8 -*-


def generate_generator():
    state = {'y': 0}

    def next_id():
        state['y'] += 1
        return state['y']
    return next_id


def generate_generator2():
    class Context:
        y = 0

    def next_id():
        Context.y += 1
        return Context.y
    return next_id


class Nonlocals(object):
    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)


def generate_generator3():
    nl = Nonlocals(y=0)

    def next_id():
        nl.y += 1
        return nl.y
    return next_id


def generate_generator4():
    """ function attribute.
    :return:
    """
    def next_id():
        next_id.y += 1
        return next_id.y
    next_id.y = 0
    return next_id
