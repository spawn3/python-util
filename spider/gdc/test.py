#!/usr/bin/env python
# coding=utf-8
from task.audio.spiderMengfou import SpiderMoe
from task.audio.spiderWangyi import Spider163

from task.comic.spiderManjia import SpiderDmzj

from task.video.spiderAcfun import SpiderAcfun
from task.video.spiderAiqiyi import SpiderIqiyi
from task.video.spiderBili import SpiderBilibili
from task.video.spiderYouku import SpiderYouku

if __name__ == '__main__':

    spider = SpiderMoe(worknum=2, queuetype='R', worktype='THREAD')
    spider.fetchDatas('www', 0, 'http://api.moefou.org/wikis.json?wiki_type=music&initial=&tag=&wiki_id=&api_key={{api_key}}&page=1')
    spider.statistic()

    # spider = Spider163(worknum=2, queuetype='R', worktype='THREAD')
    # spider.fetchDatas('www', 0)
    # spider.statistic()

    # spider = SpiderDmzj(worknum=2, queuetype='R', worktype='THREAD')
    # spider.fetchDatas('www', 0, 'http://m.dmzj.com/classify.html')
    # spider.statistic()

    # spider = SpiderAcfun(worknum=2, queuetype='R', worktype='THREAD')
    # spider.fetchDatas('album', 0, 'http://www.acfun.tv/')
    # spider.statistic()

    # spider = SpiderIqiyi(worknum=2, queuetype='R', worktype='THREAD')
    # spider.fetchDatas('www', 0, 'http://list.iqiyi.com/www/4/-------------4-1-1-iqiyi--.html')
    # spider.statistic()

    # spider = SpiderBilibili(worknum=2, queuetype='R', worktype='THREAD')
    # spider.fetchDatas('www', 0, 'http://www.bilibili.com/html/js/types.json')
    # spider.statistic()
    
    # spider = SpiderYouku(worknum=2, queuetype='R', worktype='THREAD')
    # spider.fetchDatas('www', 0)
    # spider.statistic()