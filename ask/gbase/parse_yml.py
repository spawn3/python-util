#!/usr/bin/env python
# -*- coding: utf-8 -*-


from pprint import pprint
import yaml


# pip install PyYAML


def parse_gbase_yml():
    with open('gbase.yml') as f:
        pprint(yaml.load(f))


if __name__ == '__main__':
    parse_gbase_yml()
