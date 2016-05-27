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
from webcrawl.request import Fakerequest
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

iqiyi_re = re.compile('aid.*"')

def seconds(tl):
    assert len(tl) < 3
    num = 0
    for index, one in enumerate(tl[::-1]):
        num += pow(60, index) * int(one)
    return num

class SpiderIqiyi(SpiderVideoOrigin):

    """
       爱奇艺官网 数据爬虫
    """

    def __init__(self, worknum=6, queuetype='P', worktype='COROUTINE', timeout=-1, tid=0):
        super(SpiderIqiyi, self).__init__(worknum=worknum, queuetype=queuetype, worktype=worktype, timeout=timeout, tid=tid)
        self.clsname = self.__class__.__name__
        self.headers = {"Accept":"text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
            "Accept-Language":"en-US,en;q=0.8",
            "Cache-Control":"max-age=0",
            "Connection":"keep-alive",
            "User-Agent":"Mozilla/5.0 (Linux; Android 4.2.2; GT-I9505 Build/JDQ39) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.59 Mobile Safari/537.36"
        }
        self.end = datetime.now()
        self.begin = self.end - timedelta(days=7)
        self.script = 'function(){Q.video.auth({uid: $.parseJSON($.cookie.get("P00002") || "{}").uid, platForm: "h5", rate: 1, tvid: Q.PageInfo.playInfo.tvid, vid: Q.PageInfo.playInfo.vid, cupid: Q.PageInfo.playInfo.ADPlayerID, type: Q.PageInfo.playInfo.videoFormat, qyid: $.cookie.get("QC006"), nolimit: $.cookie.get("QC004") === "0" ? 1 : 0, agenttype: Zepto.os.ios ? 12 : 13 }, { "complete":function(data){}, "success":function(data){ $("#video").attr("src", data.src); }}); return $("#video").attr("src");}'

    @store(withData(datacfg.W), Data.insert, update=True, method='MANY')
    @timelimit(3)
    @index('url')
    def fetchDetail(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        cat = additions['cat']
        tag = additions['tag']

        if 'cache.video.iqiyi.com' in url:
            outid = url[url.rindex('/')+1:url.rindex('.')].replace('av', '')
        else:
            result = requGet(url, format='HTML')
            outid = getHtmlNodeContent(result.find('.//div[@id="upDownWrap"]'), {'ATTR':'data-qpaid'})
            url = 'http://cache.video.iqiyi.com/jp/avlist/%s/1/' % outid

        browse = Fakerequest('http://localhost:12306', javascript={'end':self.script}, wait=2)
        page_result = requGet(url, dirtys=[('var tvInfoJs=', '')], format='JSON')
        if int(page_result['data']['pg']) < page_result['data']['pgt']:
            index = url.split('/')
            index[-2] = str(int(index[-2]) + 1)
            nextpage = '/'.join(index)
        else:
            nextpage = None
        yield nextpage

        parent_page_id = additions.get('parent_page_id')
        if additions.get('parent_page_id') is None:
            parent_page_id = hash(page_result['data']['vlist'][0]['vurl'])
            additions['parent_page_id'] = parent_page_id

        format = 'mp4'
        size = 0
        for one in page_result['data']['vlist']:
            # data_result = requGet(one['vurl'], headers=self.headers, timeout=10, dirtys=[('\r\n', ''),('\n', ''),('\\n', ''),('\\', ''),('\x1b', ''),('\x16', '')], format='HTML', browse=browse)
            # url = getHtmlNodeContent(data_result.find('.//video[@id="video"]'), {'ATTR':'src'})
            # if 'data.video.qiyi.com/videos/other' in url:
            #     url = ''
            url = ''
            page_url = one['vurl'].replace('www.iqiyi.com', 'm.iqiyi.com')
            during = one['timeLength']
            name = one['shortTitle']
            desc = one['vt']
            cover = one['vpic']
            author = ''
            owner = {}
            owner['avatar'] = 'http://www.qiyipic.com/common/fix/index_images/logo110x36_new.png'
            owner['name'] = '爱奇艺'
            owner['url'] = 'http://www.iqiyi.com'
            snum = one['pd']
            src = '爱奇艺'
            host = 'www.iqiyi.com'
            page_id = hash(page_url)
            atime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(one['publishTime']/1000))

            data = Data(cat=cat, url=url, format=format,
                size=size, during=during, tag=tag, name=name,
                desc=desc, cover=cover, author=author,
                owner=owner, snum=snum,
                src=src, host=host, page_url=page_url,
                page_id=page_id, parent_page_id=parent_page_id,
                atime=atime, tid=self.tid)

            yield data

    @next(fetchDetail)
    @timelimit(20)
    @index('url')
    def fetchList(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        result = requGet(url, headers=self.headers, timeout=timeout, format='HTML')
        videos = result.findall('.//div[@class="wrapper-piclist"]//li')
        if len(videos) < 30:
            nextpage = None
        else:
            index = url.split('-')
            index[-5] = str(int(index[-5]) + 1)
            # if index[-5] >5:
            #     nextpage = None
            # else:
            #     index[-5] = str(index[-5])
            nextpage = '-'.join(index)
        yield nextpage
        for one in videos:
            url = getHtmlNodeContent(one.find('.//div[@class="mod-listTitle_left"]//a'), {'ATTR':'href'}).replace('www.iqiyi.com', 'm.iqiyi.com')
            url_result = requGet(url, headers=self.headers, timeout=timeout, format='HTML')
            aidtxt = ''.join(getHtmlNodeContent(one, 'TEXT') for one in url_result.findall('.//script'))
            aid = iqiyi_re.search(aidtxt).group().replace(" ", "").replace('"', '').replace(':', '').replace('aid', '')
            url = 'http://cache.video.iqiyi.com/jp/avlist/%s/1/' % aid
            name = getHtmlNodeContent(one.find('.//div[@class="mod-listTitle_left"]//a'), 'TEXT')
            tag = [getHtmlNodeContent(one.find('.//div[@class="role_info"]'), 'TEXT').replace('\n', '').replace(' ', '')]
            additions['cat'] = ''
            yield {'url': url, 'additions': {'cat':additions['cat'], 'name':name, 'tag':tag}}

    @next(fetchList)
    @timelimit(20)
    @initflow('www')
    def fetchCat(self, url, additions={}, timeout=TIMEOUT, implementor=None):
        result = requGet(url, headers=self.headers, timeout=timeout, format='HTML')
        for cat in result.findall('.//ul[@class="mod_category_item"]//li'):
            cat_name = getHtmlNodeContent(cat, 'TEXT')
            if cat_name == '全部':
                continue
            else:
                url = 'http://list.iqiyi.com%s' % getHtmlNodeContent(cat.find('.//a'), {'ATTR':'href'})
                yield {'url':url, 'additions':{'cat':['动漫', cat_name]}}

    @next(fetchDetail)
    @timelimit(20)
    @initflow('spec')
    def fetchSpec(self, additions={}, timeout=TIMEOUT, implementor=None):
        albums = [
        'http://v.youku.com/x_getAjaxData?md=showlistnew&vid=107240046',
        'http://v.youku.com/x_getAjaxData?md=showlistnew&vid=107121098',
        'http://v.youku.com/x_getAjaxData?md=showlistnew&vid=338000877',
        ]
        # for index, one in enumerate(albums):
        #     yield {'url': one, 'additions': {'cat':['漫画', '美少女', '摇曳百合第%d季' % (index+1)]}}
        yield {'url':'http://www.iqiyi.com/a_19rrgi7t01.html', 'additions': {'cat':['动漫', '中国', '鬼神'], 'name':'中国惊奇先生', 'tag':['原创', '侦探', '都市', '讽刺', '时事']}}

if __name__ == '__main__':

    print 'start'
    spider = SpiderIqiyi(worknum=6, queuetype='P', worktype='THREAD')
    spider.fetchDatas('www', 0, 'http://list.iqiyi.com/www/4/-------------4-1-1-iqiyi--.html')
    spider.statistic()
    print 'end'
