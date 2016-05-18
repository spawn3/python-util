#!/usr/bin/env python
# -*- coding: utf-8 -*-


import socket


def test_file():
    with open('io.py') as f:
        for line in f:
            line = line.strip('\n')
            print line
