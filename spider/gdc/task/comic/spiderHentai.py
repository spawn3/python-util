#!/usr/bin/env python
# coding=utf-8
import os, re, copy, json, time, random
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
from comicspider import Data
from comicspider import TIMEOUT
from comicspider import SpiderComicOrigin
from bson import ObjectId
from webcrawl.request import FILE

import lxml.etree as ET

FILE.dir = '/home/yada/img/'

#_print, logger = logprint(modulename(__file__), modulepath(__file__))

dmzj_re = re.compile('initIntroData\(\[.*\]\);')

UA = [
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.80 Safari/537.36",
    "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_8; en-us) AppleWebKit/534.50 (KHTML, like Gecko) Version/5.1 Safari/534.50 safari 5.1 – Windows",
    "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-us) AppleWebKit/534.50 (KHTML, like Gecko) Version/5.1 Safari/534.50 IE 9.0",
    "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0; IE 8.0",
    "Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0; Trident/4.0) IE 7.0",
    "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0) IE 6.0",
    "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1) Firefox 4.0.1 – MAC",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.6; rv:2.0.1) Gecko/20100101 Firefox/4.0.1 Firefox 4.0.1 – Windows",
    "Mozilla/5.0 (Windows NT 6.1; rv:2.0.1) Gecko/20100101 Firefox/4.0.1 Opera 11.11 – MAC",
    "Opera/9.80 (Macintosh; Intel Mac OS X 10.6.8; U; en) Presto/2.8.131 Version/11.11 Opera 11.11 – Windows",
    "Opera/9.80 (Windows NT 6.1; U; en) Presto/2.8.131 Version/11.11 Chrome 17.0 – MAC",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_0) AppleWebKit/535.11 (KHTML, like Gecko) Chrome/17.0.963.56 Safari/535.11",
    "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Maxthon 2.0)TT",
    "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; TencentTraveler 4.0) The World 2.x",
    "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1) The World 3.x",
    "?Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; The World)",
    "?Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Trident/4.0; SE 2.X MetaSr 1.0; SE 2.X MetaSr 1.0; .NET CLR 2.0.50727; SE 2.X MetaSr 1.0) 360SE",
    "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; 360SE) Avant",
    "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Avant Browser) Green Browser",
    "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)"
]

SPAN = [
3, 4, 3, 4, 5, 6, 2, 7, 3, 4, 4, 5, 7, 2, 7, 7
]

class SpiderEhentai(SpiderComicOrigin):

    """
       哔哩官网 数据爬虫
    """

    def __init__(self, worknum=6, queuetype='P', worktype='COROUTINE', timeout=-1, tid=0):
        super(SpiderEhentai, self).__init__(worknum=worknum, queuetype=queuetype, worktype=worktype, timeout=timeout, tid=tid)
        self.clsname = self.__class__.__name__
        self.headers = {}
        self.end = datetime.now()
        self.begin = self.end - timedelta(days=7)

    @store(withData(datacfg.W), Data.insert, update=True, method='MANY')
    @timelimit(3)
    @index('url')
    def fetchDetail(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        headers = {'Host': 'g.e-hentai.org',
             'Accept-Language': 'en-US,en;q=0.8',
             'Accept-Encoding': 'gzip, deflate, sdch',
             'Cache-Control': 'max-age=0',
             'Host': 'g.e-hentai.org',
             'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8',
             'Upgrade-Insecure-Requests': '1'}
        headers['User-Agent'] = random.choice(UA)
        time.sleep(random.choice(SPAN))
        result = requGet(url, headers=dict(headers, **{'Referer':url}), timeout=timeout, format='HTML')
        comics = result.findall('.//div[@id="gdt"]//div[@class="gdtm"]//a')

        if len(comics) < 40:
            nextpage = None
        else:
            index = url.split('=')
            index[-1] = int(index[-1]) + 1
            # if index[-1] >5:
            #     nextpage = None
            # else:
            #     index[-1] = str(index[-1])
            #     nextpage = '='.join(index)
            index[-1] = str(index[-1])
            nextpage = '='.join(index)
        yield nextpage

        info = result.findall('.//div[@id="gdd"]//table//tr')

        cat = ['doujin', 'hentai']
        tag = [getHtmlNodeContent(one, 'TEXT') for one in result.findall('.//div[@id="taglist"]//a')]
        name = getHtmlNodeContent(result.find('.//h1[@id="gn"]'), 'TEXT').strip()
        desc = getHtmlNodeContent(result.find('.//h1[@id="gj"]'), 'TEXT')
        cover = getHtmlNodeContent(result.find('.//div[@id="gd1"]//img'), {'ATTR':'src'})
        author = ''
        atime = None
        owner = {}
        owner['url'] = getHtmlNodeContent(result.find('.//div[@id="gdn"]//a'), 'TEXT')
        headers['User-Agent'] = random.choice(UA)
        time.sleep(random.choice(SPAN))
        user_result = requGet(getHtmlNodeContent(result.findall('.//div[@id="gdn"]//a')[-1], {'ATTR':'href'}), headers=headers, timeout=timeout, format='HTML')
        owner['name'] = getHtmlNodeContent(user_result.find('.//div[@id="profilename"]//font'), 'TEXT')
        # owner['avatar'] = getHtmlNodeContent(user_result.findall('.//table[@class="ipbtable"]//tr//td//div')[1].find('.//img'), {'ATTR':'src'})
        owner['avatar'] = ''
        relate_page = {}
        snum = 0
        src = 'e-hentai'
        host = 'g.e-hentai.org'
        language = ''
        parody = ''

        format = 'image'

        for one in info:
            if getHtmlNodeContent(one.find('.//td[@class="gdt1"]'), 'TEXT') == 'Posted:':
                atime = datetime.strptime(getHtmlNodeContent(one.find('.//td[@class="gdt2"]'), 'TEXT')+':00', '%Y-%m-%d %H:%M:%S')
            if getHtmlNodeContent(one.find('.//td[@class="gdt1"]'), 'TEXT') == 'Parent:':
                phref = getHtmlNodeContent(one.find('.//td[@class="gdt2"]//a'), {'ATTR':'href'})
                if phref and not phref == 'None:':
                    ppage = phref + '?p=0'
                    ppid = str(hash(ppage))
                    relate_page = {ppid:ppage}
            if getHtmlNodeContent(one.find('.//td[@class="gdt1"]'), 'TEXT') == 'Language:':
                language = getHtmlNodeContent(one.find('.//td[@class="gdt2"]'), 'TEXT').split(' ')[0].strip()

        info = result.findall('.//div[@id="taglist"]//table//tr')
        for one in info:
            if getHtmlNodeContent(one.find('.//td[@class="tc"]'), 'TEXT') == 'parody:':
                parody = getHtmlNodeContent(one.find('.//div[@class="gt"]//a'), 'TEXT')
            if getHtmlNodeContent(one.find('.//td[@class="tc"]'), 'TEXT') == 'artist:':
                author = getHtmlNodeContent(one.find('.//div[@class="gt"]//a'), 'TEXT')

        parent_page_id = hash(url)
        parent_id = url.replace('http://g.e-hentai.org/g/', '').split('/')[0]
        if not os.path.exists(os.path.join(FILE.dir, '%s/' % src)):
            os.makedirs(os.path.join(FILE.dir, '%s/' % src))

        for index, page in enumerate(comics):
            page_url = getHtmlNodeContent(page, {'ATTR':'href'})
            page_id = hash(page_url)
            time.sleep(random.choice(SPAN))
            headers['User-Agent'] = random.choice(UA)
            time.sleep(random.choice(SPAN))
            page_result = requGet(page_url, headers=dict(headers, **{'Referer':url}), timeout=timeout, format='HTML')
            srcs = page_result.findall('.//img')
            img_url = ''
            download = False
            snum = int(page_url.split('-')[-1])
            for one in srcs:
                img_url = getHtmlNodeContent(one, {'ATTR':'src'})
                if 'http://ehgt.org/g' in img_url:
                    img_url = ''
                else:
                    download = True
                    break
            if img_url:
                headers['User-Agent'] = random.choice(UA)
                # time.sleep(random.choice(SPAN))
                try:
                    requGet(img_url, headers=dict(headers, **{'Referer':url}), timeout=timeout, format='TEXT', filepath='%s/%s-%s' % (src, parent_id, str(snum)))
                except:
                    download = False
            page_data = Data(cat=cat, url=None, tag=tag, name=name,
                desc=desc, cover=cover, author=author,
                owner=owner, snum=snum,
                src=src, host=host, language=language, parody=parody, relate_page=relate_page, page_url=page_url,
                page_id=page_id, parent_page_id=parent_page_id,
                atime=atime, tid=self.tid, download=download)
            yield page_data

    @next(fetchDetail)
    @timelimit(20)
    @index('url')
    @initflow('www')
    def fetchList(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        headers = {'Host': 'g.e-hentai.org',
             'Accept-Language': 'en-US,en;q=0.8',
             'Accept-Encoding': 'gzip, deflate, sdch',
             'Cache-Control': 'max-age=0',
             'Host': 'g.e-hentai.org',
             'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8',
             'Upgrade-Insecure-Requests': '1'}
        headers['User-Agent'] = random.choice(UA)
        headers['Referer'] = url
        time.sleep(random.choice(SPAN))
        result = requGet(url, headers=headers, timeout=timeout, format='HTML')
        comics = result.findall('.//div[@class="it5"]')
        if len(comics) < 15:
            nextpage = None
        else:
            index = url.split('/')
            index[-1] = int(index[-1]) + 1
            # if index[-1] >5:
            #     nextpage = None
            # else:
            #     index[-1] = str(index[-1])
            #     nextpage = '/'.join(index)
            index[-1] = str(index[-1])
            nextpage = '/'.join(index)
        yield nextpage
        for one in comics:
            url = getHtmlNodeContent(one.find('.//a'), {'ATTR':'href'}) + '?p=0'
            yield {'url':url}


if __name__ == '__main__':

    print 'start'
    spider = SpiderEhentai(worknum=6, queuetype='P', worktype='COROUTINE')
    spider.fetchDatas('www', 0, 'http://g.e-hentai.org/non-h/0')
    spider.statistic()
    print 'end'
