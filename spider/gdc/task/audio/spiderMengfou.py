#!/usr/bin/python
# coding=utf-8
import copy, time
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
from audiospider import Data
from audiospider import TIMEOUT
from audiospider import SpiderAudioOrigin

#_print, logger = logprint(modulename(__file__), modulepath(__file__))

def seconds(tl):
    assert len(tl) < 3
    num = 0
    for index, one in enumerate(tl[::-1]):
        num += pow(60, index) * int(one)
    return num

class SpiderMoe(SpiderAudioOrigin):

    """
       萌否官网 数据爬虫
    """

    def __init__(self, worknum=6, queuetype='P', worktype='COROUTINE', timeout=-1, tid=0):
        super(SpiderMoe, self).__init__(worknum=worknum, queuetype=queuetype, worktype=worktype, timeout=timeout, tid=tid)
        self.clsname = self.__class__.__name__
        self.api_key = '3e304078c769743445311c894eb221d90566aa33b'

#    @store(withData(datacfg.W), Data.insert, update=True, method='MANY')
    @timelimit(3)
    def fetchDetail(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        result = requGet(url, timeout=timeout, format='JSON')
        outid = url.split('=')[-1]
        url = 'http://api.moefou.org/music/detail.json?wiki_id=%s&api_key={{api_key}}'.replace('{{api_key}}', self.api_key) % outid
        album_result = requGet(url, timeout=timeout, format='JSON') or {'response':{'wiki':{'wiki_meta':[], 'wiki_cover':{'large':''}}}}
        for one in album_result['response']['wiki']['wiki_meta']:
            if one['meta_key'] == '艺术家':
                singer = one['meta_value']
        else:
            singer = ''
        pages = result['response']['subs']
        if pages:
            parent_page_id = hash('http://moe.fm/listen/h5?song=%s' % str(pages[0]['sub_id']))
            for one in pages:
                if one['sub_upload']:
                    url = one['sub_upload'][0]['up_url']
                    format = 'mp3'
                    size = one['sub_upload'][0]['up_data']['filesize']/float(1024*1024)
                    during = seconds(one['sub_upload'][0]['up_data']['time'].split(':'))
                    tag = []
                    name = one['sub_title']
                    desc = one['sub_about']
                    cover = album_result['response']['wiki']['wiki_cover']['large']
                    snum = int(one['sub_order'])
                    src = '萌否'
                    host = 'moe.fm'
                    page_url = 'http://moe.fm/listen/h5?song=%s' % str(one['sub_id'])
                    page_id = hash(page_url)
                    parent_page_id = parent_page_id
                    atime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(float(one['sub_upload'][0]['up_date'])))
                    atime = datetime.strptime(atime, '%Y-%m-%d %H:%M:%S')
                    data = Data(url=url, format=format,
                        size=size, during=during, tag=tag, name=name,
                        desc=desc, cover=cover, snum=snum, singer=singer,
                        src=src, host=host, page_url=page_url,
                        page_id=page_id, parent_page_id=parent_page_id,
                        atime=atime, tid=self.tid)
                    print data
                    yield data

    @next(fetchDetail)
    @initflow('www')
    @timelimit(20)
    @index('url')
    def fetchList(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        url = url.replace('{{api_key}}', self.api_key)
        result = requGet(url, timeout=timeout, format='JSON')
        audios = result['response']['wikis']
        if len(audios) < 20:
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
        for one in audios:
            yield {'url': 'http://api.moefou.org/music/subs.json?sub_type=song,ep&api_key={{api_key}}&wiki_id=%s'.replace('{{api_key}}', self.api_key) % str(one['wiki_id'])}

if __name__ == '__main__':

    print 'start'
    spider = SpiderMoefou(worknum=6, queuetype='P', worktype='THREAD')
    spider.fetchDatas('www', 0, 'http://api.moefou.org/wikis.json?wiki_type=music&initial=&tag=&wiki_id=&api_key={{api_key}}&page=1')
    spider.statistic()
    print 'end'
