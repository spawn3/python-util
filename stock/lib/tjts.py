#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, sys
import argparse
from prettytable import PrettyTable


def stock_list(begin_price, end_price, factor=0.04, partition=4, money=20000):
    pt = PrettyTable(['IDX', 'Buy', 'Sell', 'Diff', 'Diff/4', 'Partition', 'Money', 'Stock1', "Stock2"])
    pt.hrules = 1

    price_list = []
    price = begin_price
    while price < end_price:
        price_list.append(price)
        price = price * (1 + factor)

    for i, price in enumerate(price_list):
        stock = int(money / price)
        buy_sell_list = []
        for j in range(partition):
            price1 = price * (1 + factor * j / partition)
            price2 = price * (1 + factor * (j + 1) / partition)
            line = u'{:.2f}-{:.2f}'.format(price1, price2)
            buy_sell_list.append(line)
        pt.add_row([i,
                    u'{:.2f}'.format(price),
                    u'{:.2f}'.format(price * (1 + factor)),
                    u'{:.2f}'.format(price * factor),
                    u'{:.2f}'.format(price * factor / partition),
                    '\n'.join(buy_sell_list),
                    money,
                    stock,
                    stock / 100 * 100])

    print(pt)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()

    def _stock_list(args):
        stock_list(args.begin, args.end, factor=args.factor * 0.01, partition=args.partition, money=args.money)

    parser_stock_List = subparsers.add_parser('stock', help='list stock')
    parser_stock_List.add_argument('-b', '--begin', required=False, type=float, default=20.98, help="begin_price")
    parser_stock_List.add_argument('-e', '--end', required=False, type=float, default=36, help="end_price")
    parser_stock_List.add_argument('-f', '--factor', required=False, type=float, default=4, help="factor")
    parser_stock_List.add_argument('-p', '--partition', required=False, type=int, default=4, help="partition")
    parser_stock_List.add_argument('-m', '--money', required=False, type=float, default=20000, help="money")
    parser_stock_List.set_defaults(func=_stock_list)

    args = parser.parse_args()
    args.func(args)
