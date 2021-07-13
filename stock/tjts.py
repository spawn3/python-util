#!/usr/bin/env python

import os, sys
import argparse
from prettytable import PrettyTable


def stock_list(begin_price, end_price, factor=0.0416, money=20000):
    pt = PrettyTable(['IDX', 'Buy', "Sell", 'Diff', 'Money', 'Stock1', "Stock2"])
    pt.hrules = 1

    price_list = []
    price = begin_price
    while price < end_price:
        price_list.append(price)
        price = price * (1 + factor)

    for i, price in enumerate(price_list):
        stock = int(money / price)
        pt.add_row([i,
                    '%.2f' % price,
                    '%.2f' % (price * (1 + factor)),
                    '%.2f' % (price * factor),
                    money,
                    stock,
                    stock / 100 * 100])

    print(pt)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()

    def _stock_list(args):
        stock_list(args.begin, args.end, factor=args.factor, money=args.money)

    parser_stock_List = subparsers.add_parser('stock', help='list stock')
    parser_stock_List.add_argument('-b', '--begin', required=False, type=float, default=20.98, help="begin_price")
    parser_stock_List.add_argument('-e', '--end', required=False, type=float, default=36, help="end_price")
    parser_stock_List.add_argument('-f', '--factor', required=False, type=float, default=0.0416, help="factor")
    parser_stock_List.add_argument('-m', '--money', required=False, type=float, default=20000, help="money")
    parser_stock_List.set_defaults(func=_stock_list)

    args = parser.parse_args()
    args.func(args)
