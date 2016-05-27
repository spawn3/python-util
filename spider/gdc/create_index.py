#!/usr/bin/python

#-*- coding:utf-8 -*-
from pymongo import (
    MongoReplicaSetClient,
    MongoClient,
    read_preferences
)

mc = MongoClient(host='localhost')
grab = mc['dandan-jiang']
tb_raw_video = grab['video']


def main():
    tb_raw_video.ensure_index([('parent_page_id', 1), ('snum', 1)],  background=True)

if __name__ == '__main__':
    main()