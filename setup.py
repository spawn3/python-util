#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os

from ask import __version__

try:
    from setuptools import setup, find_packages
except ImportError:
    from distutils.core import setup

f = open(os.path.join(os.path.dirname(__file__), 'README.md'))
long_description = f.read()
f.close()

setup(
    name='ask',
    version=__version__,
    description='Python client for Redis key-value store',
    long_description=long_description,
    url='http://github.com/spawn3/python-util',
    author='Andy McCurdy',
    author_email='sedrik@gmail.com',
    maintainer='Andy McCurdy',
    maintainer_email='toctls@163.com',
    keywords=['Python', 'Util'],
    license='MIT',
    packages=find_packages(),
    test_suite='tests.all_tests',
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Environment :: Console',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2.5',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.2',
        'Programming Language :: Python :: 3.3',
        ]
)
