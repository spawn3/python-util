#!/usr/bin/python
# coding=utf-8

import pymongo

from webcrawl.handleRequest import requGet

if __name__ == '__main__':
    mc = pymongo.MongoClient('localhost')
    ddj = mc['dandan-jiang']
    for one in ddj.video.find({'src':'哔哩哔哩'}):
        if not 'mp4' in one['url']:
            print one['_id']