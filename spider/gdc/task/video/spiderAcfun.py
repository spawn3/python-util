#!/usr/bin/python
# coding=utf-8
import os, re, copy, time
from pymongo import MongoClient
from datetime import timedelta
from datetime import datetime
from webcrawl.request import requGet
from webcrawl.request import requPost
from webcrawl.request import getHtmlNodeContent
from webcrawl.request import getXmlNodeContent
from webcrawl.task import retry
from webcrawl.task import index
from webcrawl.task import initflow
from webcrawl.request import getJsonNodeContent
from webcrawl.task import store
from webcrawl.task import timelimit
from webcrawl.task import next
from webcrawl.request import ensureurl
from webcrawl.request import parturl
from model.setting import withData, datacfg
from videospider import Data
from videospider import TIMEOUT
from videospider import SpiderVideoOrigin

#_print, logger = logprint(modulename(__file__), modulepath(__file__))

class SpiderAcfun(SpiderVideoOrigin):

    """
       哔哩官网 数据爬虫
    """

    def __init__(self, worknum=6, queuetype='P', worktype='COROUTINE', timeout=-1, tid=0):
        super(SpiderAcfun, self).__init__(worknum=worknum, queuetype=queuetype, worktype=worktype, timeout=timeout, tid=tid)
        self.clsname = self.__class__.__name__
        self.headers = {"Accept":"application/json, text/javascript, */*; q=0.01",
            "Accept-Encoding":"gzip, deflate, sdch",
            "Accept-Language":"en-US,en;q=0.8",
            "Cache-Control":"max-age=0",
            "Connection":"keep-alive",
            "Host":"www.bilibili.com",
            "User-Agent":"Mozilla/5.0 (Linux; Android 5.1.1; Nexus 6 Build/LYZ28E) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.20 Mobile Safari/537.36",
            "X-Requested-With":"XMLHttpRequest"}
        self.end = datetime.now()
        self.begin = self.end - timedelta(days=7)

    @store(withData(datacfg.W), Data.insert, update=True, method='MANY')
    @timelimit(3)
    def fetchSoloDetail(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        cat = additions['cat']        
        outid = url[url.rindex('/')+1:].replace('ac', '')
        headers = {
            "deviceType":"2", 
            "Referer":"http://m.acfun.tv/player?date=undefined", 
            "User-Agent":"Mozilla/5.0 (iPhone; CPU iPhone OS 8_0 like Mac OS X) AppleWebKit/600.1.3 (KHTML, like Gecko) Version/8.0 Mobile/12A4345d Safari/600.1.4"
        }
        url = 'http://api.acfun.tv/apiserver/content/info?contentId=%s' % outid
        info_result = requGet(url, timeout=30, format='JSON')
        page_url = url
        url = 'http://api.aixifan.com/plays/%s/realSource' % str(info_result['data']['fullContent']['videos'][0]['videoId'])
        page_url = page_url + '#' + str(info_result['data']['fullContent']['videos'][0]['videoId'])
        url_result = requGet(url, headers=headers, format='JSON')
        url = url_result['data']['files'][0]['url']
        if type(url) == list:
            url.append('')
            url = url[0]
        format = 'mp4'

        size = 0
        during = info_result['data']['fullContent']['videos'][0]['time']
        tag = info_result['data']['fullContent']['tags']
        name = info_result['data']['fullContent']['videos'][0]['name']
        desc = info_result['data']['fullContent']['description']
        cover = info_result['data']['fullContent']['cover']
        author = ''

        owner = {'avatar':'', 'name':'', 'url':''}
        owner['avatar'] = info_result['data']['fullContent']['user']['userImg']
        owner['name'] = info_result['data']['fullContent']['user']['username']
        owner['url'] = 'http://www.acfun.tv/u/%s.aspx' % str(info_result['data']['fullContent']['user']['username'])
        snum = 0

        src = 'Acfun'
        host = 'www.acfun.tv'

        page_id = hash(page_url)
        parent_page_id = None

        atime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(info_result['data']['fullContent']['releaseDate']/1000.0))

        data = Data(cat=cat, url=url, format=format,
            size=size, during=during, tag=tag, name=name,
            desc=desc, cover=cover, author=author,
            owner=owner, snum=snum,
            src=src, host=host, page_url=page_url,
            page_id=page_id, parent_page_id=parent_page_id,
            atime=atime, tid=self.tid)
        yield data


    @next(fetchSoloDetail)
    @timelimit(20)
    @index('url')
    def fetchSoloList(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        info_result = requGet(url, timeout=180, format='HTML')
        videos = info_result.xpath('//div[starts-with(@class,"unit unit")]')
        if len(videos) < additions['pagesize']:
            nextpage = None
        else:
            page = int(url[url.rindex('/')+1:url.rindex('.')])+1
            if page > 5:
                nextpage = None
            else:
                nextpage = url[:url.rindex('/')+1] + str(page) + url[url.rindex('.'):]
        yield nextpage
        for one in videos:
            yield {'url': 'http://www.acfun.tv%s' % getHtmlNodeContent(one.find('.//a[@class="title"]'), {'ATTR':'href'}), 'additions': {"cat":additions['cat']}}

    @next(fetchSoloList)
    @timelimit(20)
    @initflow('solo')
    def fetchSoloCat(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        info_result = requGet('http://www.acfun.tv/', timeout=timeout, format='HTML')
        cats = {}
        pagesize = 20
        for upcat in info_result.findall('.//div[@id="guide-bar"]//a'):
            url = getHtmlNodeContent(upcat, {'ATTR':'href'})
            cats[getHtmlNodeContent(upcat, {'ATTR':'data-channel'})] = {'url':url, 'txt':getHtmlNodeContent(upcat, 'TEXT')}
        for downcat in info_result.findall('.//div[@id="sub-guide-inner"]//div'):
            channel = getHtmlNodeContent(downcat, {'ATTR':'class'})
            channel = channel.split(' ')[1].replace('channel-', '')
            cat = [cats[channel]['txt']] if channel in cats else []
            for e in downcat.findall('.//a'):
                channel_id = getHtmlNodeContent(e, {'ATTR':'href'}).split('/')[2].replace('list', '')
                if channel_id.isdigit():
                    yield {'url':'http://www.acfun.tv/dynamic/channel/1.aspx?channelId=%s&orderBy=0&pageSize=%d' % (channel_id, pagesize), 'additions':{'pagesize':pagesize, 'cat':[getHtmlNodeContent(e, 'TEXT')] + cat}}

    @store(withData(datacfg.W), Data.insert, update=True, method='MANY')
    @timelimit(3)
    def fetchAlbumDetail(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        cat = additions['cat']        
        outid = url[url.rindex('/')+1:].replace('ab', '')
        headers = {
            "deviceType":"2", 
            "Referer":"http://m.acfun.tv/player?date=undefined", 
            "User-Agent":"Mozilla/5.0 (iPhone; CPU iPhone OS 8_0 like Mac OS X) AppleWebKit/600.1.3 (KHTML, like Gecko) Version/8.0 Mobile/12A4345d Safari/600.1.4"
        }
        url = 'http://www.acfun.tv/bangumi/bangumi/info?id=%s' % outid
        info_result = requGet(url, timeout=30, format='JSON')
        pages = info_result['data']['videos']
        pages = sorted(pages, key=lambda v:v['sort'])
        if pages:
            first = pages[0]
            parent_page_id = hash('http://www.acfun.tv/v/ab%s_1' % first['bangumiId'])
            tag = [one['name'] for one in info_result['data']['tags']]
            for index, one in enumerate(pages):
                page_url = 'http://www.acfun.tv/v/ab%s_%d#%s' % (first['bangumiId'], index+1, one['danmakuId'])
                url = 'http://api.aixifan.com/plays/%s/realSource' % str(one['danmakuId'])
                url_result = requGet(url, headers=headers, format='JSON')
                try:
                    url = url_result['data']['files'][0]['url']
                    if type(url) == list:
                        url.append('')
                        url = url[0]
                except:
                    url = ''
                format = 'mp4'

                size = 0
                during = one['time']
                
                name = ''.join([info_result['data']['title'], '-', one['title']])
                desc = info_result['data']['intro']
                cover = info_result['data']['cover']
                author = ''

                owner = {'avatar':'', 'name':'', 'url':''}
                owner['avatar'] = info_result['data']['contributors'][0]['avatar']
                owner['name'] = info_result['data']['contributors'][0]['name']
                owner['url'] = 'http://www.acfun.tv/u/%s.aspx' % str(info_result['data']['contributors'][0]['id'])
                snum = index + 1

                src = 'Acfun'
                host = 'www.acfun.tv'

                page_id = hash(page_url)

                atime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(info_result['data']['lastUpdateTime']/1000.0))

                data = Data(cat=cat, url=url, format=format,
                    size=size, during=during, tag=tag, name=name,
                    desc=desc, cover=cover, author=author,
                    owner=owner, snum=snum,
                    src=src, host=host, page_url=page_url,
                    page_id=page_id, parent_page_id=parent_page_id,
                    atime=atime, tid=self.tid)
                yield data

    @next(fetchAlbumDetail)
    @timelimit(20)
    @index('url')
    def fetchAlbumList(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        info_result = requGet(url, timeout=180, format='JSON')
        videos = info_result['data']['list']
        if len(videos) < additions['pagesize']:
            nextpage = None
        else:
            index = url.split('=')
            index[-1] = int(index[-1]) + 1
            if index[-1] > 5:
                nextpage = None
            else:
                index[-1] = str(index[-1])
                nextpage = '='.join(index)
        yield nextpage
        for one in videos:
            yield {'url': 'http://www.acfun.tv/v/ab%s' % str(one['id']), 'additions': {"cat":additions['cat']}}

    @next(fetchAlbumList)
    @timelimit(20)
    @initflow('album')
    def fetchAlbumCat(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        info_result = requGet('http://www.acfun.tv/', timeout=timeout, format='HTML')
        pagesize = 42
        for upcat in info_result.findall('.//div[@id="guide-bar"]//a'):
            url = getHtmlNodeContent(upcat, {'ATTR':'href'})
            cat_txt = getHtmlNodeContent(upcat, 'TEXT')
            if 'list144' in url:
                cat_result = requGet('http://www.acfun.tv%s' % url, timeout=timeout, format='HTML')
                for downcat in cat_result.findall('.//ul[@id="filter-type"]//li'):
                    cat_type = getHtmlNodeContent(downcat, {'ATTR':'data-value'})
                    url = 'http://www.acfun.tv/bangumi/bangumi/page?pageSize=%d&isWeb=1&&sort=1&type=%s&pageNo=1' % (pagesize, cat_type)
                    yield {'url':url, 'additions':{'pagesize':pagesize, 'cat':[cat_txt, getHtmlNodeContent(downcat, 'TEXT')]}}


if __name__ == '__main__':

    print 'start'
    spider = SpiderAcfun(worknum=6, queuetype='P', worktype='THREAD')
    spider.fetchDatas('album', 0, 'http://www.acfun.tv/')
    spider.statistic()
    print 'end'