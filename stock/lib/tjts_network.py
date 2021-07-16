#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, sys
import argparse
from prettytable import PrettyTable


def stock_list(begin_price, end_price, factor=0.04, partition=4, money=20000):
    pt = PrettyTable(['IDX', 'Buy', 'Diff', '/4', 'Money', 'Stock1', "Stock2"])
    pt.hrules = 1

    factor = 0.01

    price_list = []
    price = begin_price
    while price < end_price:
        price_list.append(price)
        price = price * (1 + factor)

    for i, price in enumerate(price_list):
        stock = int(money / price)
        xs = []
        for j in [4]:
            x = '{}:{:.2f}'.format(j, 0.0 if i == 0 else (price - price_list[i - j])) if i % j == 0 else ''
            xs.append(x)

        lst = [i,
               u'TJ-{:.2f}'.format(price),
               u'{:.2f}'.format(price * factor)
               ]

        lst.extend(xs)

        lst.extend([money,
                    stock,
                    stock / 100 * 100])

        pt.add_row(lst)

    print(pt)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()

    def _stock_list(args):
        stock_list(args.begin, args.end, factor=args.factor * 0.01, partition=args.partition, money=args.money)

    parser_stock_List = subparsers.add_parser('stock', help='list stock')
    parser_stock_List.add_argument('-b', '--begin', required=False, type=float, default=1, help="begin_price")
    parser_stock_List.add_argument('-e', '--end', required=False, type=float, default=30, help="end_price")
    parser_stock_List.add_argument('-f', '--factor', required=False, type=float, default=4, help="factor")
    parser_stock_List.add_argument('-p', '--partition', required=False, type=int, default=4, help="partition")
    parser_stock_List.add_argument('-m', '--money', required=False, type=float, default=20000, help="money")
    parser_stock_List.set_defaults(func=_stock_list)

    args = parser.parse_args()
    args.func(args)
