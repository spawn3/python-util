#!/usr/bin/env python
# coding=utf-8
import os, re, copy, json, time, commands
from pymongo import MongoClient
from datetime import timedelta
from datetime import datetime
from webcrawl.handleRequest import requGet
from webcrawl.handleRequest import requPost
from webcrawl.handleRequest import getHtmlNodeContent
from webcrawl.handleRequest import getXmlNodeContent
from webcrawl.work import retry
from webcrawl.work import index
from webcrawl.work import initflow
from webcrawl.handleRequest import getJsonNodeContent
from webcrawl.work import store
from webcrawl.work import timelimit
from webcrawl.work import next
from webcrawl.handleRequest import ensureurl
from webcrawl.handleRequest import parturl
from actorspider import Data
from actorspider import TIMEOUT
from actorspider import withData, RDB, WDB
from actorspider import SpiderActorOrigin
from webcrawl.character import unicode2utf8

#_print, logger = logprint(modulename(__file__), modulepath(__file__))

role_re = re.compile('<[^>]*>')

class SpiderActor(SpiderActorOrigin):

    """
       哔哩官网 数据爬虫
    """

    def __init__(self, worknum=6, queuetype='P', worktype='COROUTINE', timeout=-1, tid=0):
        super(SpiderActor, self).__init__(worknum=worknum, queuetype=queuetype, worktype=worktype, timeout=timeout, tid=tid)
        self.clsname = self.__class__.__name__
        self.headers = {}
        self.end = datetime.now()
        self.begin = self.end - timedelta(days=7)

    @store(withData(WDB), Data.insert, update=True, method='MANY')
    @timelimit(3)
    def writedata(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        data = additions['data']
        name_cn = data.get('name_ch')
        info = data.get('detail', [])
        name = data.get('name_jp')
        avatar = data.get('cover', '')
        avatar = 'http:%s' % avatar if avatar.startswith('//lain.bgm.tv') else avatar
        nick = []
        gender = ''
        birth = None
        desc = re.sub(role_re, '', data.get('dec', ''))
        job = ''
        relative = []

        refkey = 'bangumi.tv-%s' % str(url)

        tag = []
        album = []
        role = []

        for one in info:
            one = one.split(':')
            flag = True
            if '生日' in one[0]:
                birth = one[-1].strip()
                flag = False
            if '别名' in one[0]:
                nick.append(one[-1].strip())
                name = name or one[-1].strip()
                flag = False
            if '性别' in one[0]:
                gender = 'm' if '男' in one[-1] else '女'
                flag = False
            if '中文' in one[0]:
                name_cn = name_cn or one[-1].strip()
                name = name or one[-1].strip()
                flag = False
            if flag:
                tag.append(one[-1].strip())


        if name_cn:
            nick = [name_cn] + nick
            nick = list(set(nick))
        nick = ','.join(nick)

        rank = 0
        share = 0
        ncos = 0
        favor = 0
        like = 0
        unlike = 0

        uid = None
        atime = datetime.now()

        page_data = Data(name=name, avatar=avatar, nick=nick, gender=gender, birth=birth, desc=desc, job=job, relative=relative,
            refkey=refkey,
            tag=tag, album=album, role=role,
            rank=rank, share=share, ncos=ncos, favor=favor, like=like, unlike=unlike,
            uid=uid, atime=atime
            )
        yield page_data

    @next(writedata)
    @timelimit(20)
    @initflow('www')
    def readtxt(self, additions={}, timeout=TIMEOUT, implementor=None):
        with open('/Users/uni/Downloads/persons.txt', 'r') as f:
            for line in f:
                try:
                    data = eval(line)
                    data = unicode2utf8(data)
                except:
                    continue
                yield {'url':data['id'], 'additions': {'data':data}}


if __name__ == '__main__':

    print 'start'
    spider = SpiderActor(worknum=6, queuetype='P', worktype='COROUTINE')
    spider.fetchDatas('www', 0)
    spider.statistic()
    print 'end'
