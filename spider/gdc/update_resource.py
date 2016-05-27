#!/usr/bin/python
# coding=utf-8
import pymongo, requests
from bson.objectid import ObjectId
# from task.celery import app
from adesk.db.connection import create_connection
from webcrawl.handleRequest import requGet, getHtmlNodeContent

replSet = 'adesk01'
masters = ['jxqcta','jxqctb','jxqctd','jxqcte','dscnca','dscncb','dscncc']
cluster_mc = create_connection(masters, replSet=replSet)
cluster_ddj = cluster_mc['dandan_jiang']

jxqctm_mc = pymongo.MongoClient('dscncg')
jxqctm_ddj = jxqctm_mc['dandan-jiang']

TIMEOUT = 30

# @app.task
def update_resource(vid):
    video = jxqctm_ddj['video'].find_one({'_id':ObjectId(vid)})
    funs = {'哔哩哔哩':update_bili, 'Acfun':update_acfun}
    if str(video['src']) in funs and video:
        funs[str(video['src'])](video)

def update_bili(video):
    url = video['page_url']
    outid = url[url.rindex('/')+1:url.rindex('.')].replace('av', '')
    if '#page' in url:
        pagenum = url.split('page=')[-1]
        url = 'http://www.bilibili.com/m/html5?aid=%s&page=%s' % (outid, pagenum)
    else:
        url = 'http://www.bilibili.com/m/html5?aid=%s' % outid
    headers = {"Accept":"application/json, text/javascript, */*; q=0.01",
        "Accept-Encoding":"gzip, deflate, sdch",
        "Accept-Language":"en-US,en;q=0.8",
        "Cache-Control":"max-age=0",
        "Connection":"keep-alive",
        "Host":"www.bilibili.com",
        "Referer":"http//www.bilibili.com/mobile/video/av%s.html?tg" % outid,
        "User-Agent":"Mozilla/5.0 (Linux; Android 5.1.1; Nexus 6 Build/LYZ28E) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.20 Mobile Safari/537.36",
        "X-Requested-With":"XMLHttpRequest"}
    test = requests.head(video['url'], headers=headers)
    if test.status_code == 503 or test.status_code == 200:
        print '=========aaaa'
        return
    data_result = requGet(url, headers=headers, timeout=TIMEOUT, format='JSON')
    url = data_result['src']
    jxqctm_ddj['video'].update({'_id':video['_id']}, {'$set':{'url':url}})
    cluster_ddj['resource'].update({'_id':video['_id']}, {'$set':{'url':url}})

def update_acfun(video):
    url = video['page_url']
    headers = {
            "deviceType":"2", 
            "Referer":"http://m.acfun.tv/player?date=undefined", 
            "User-Agent":"Mozilla/5.0 (iPhone; CPU iPhone OS 8_0 like Mac OS X) AppleWebKit/600.1.3 (KHTML, like Gecko) Version/8.0 Mobile/12A4345d Safari/600.1.4"
        }
    outid = video['page_url'].split('#')[-1]
    url = 'http://api.aixifan.com/plays/%s/realSource' % outid
    print 'cccc', video['page_url'], url
    url_result = requGet(url, headers=headers, format='JSON')
    url = url_result['data']['files'][0]['url']
    if type(url) == list:
        url.append('')
        url = url[0]
    print 'dddd', url
    jxqctm_ddj['video'].update({'_id':video['_id']}, {'$set':{'url':url}})
    cluster_ddj['resource'].update({'_id':video['_id']}, {'$set':{'url':url}})

if __name__ == '__main__':
    for one in jxqctm_ddj['video'].find({'src':'哔哩哔哩'}).limit(1):
        print '====start bili', one['_id'], one['url']
        update_resource(one['_id'])
        one = jxqctm_ddj['video'].find_one({'_id':one['_id']})
        print '====end', one['_id'], one['url']
    for one in jxqctm_ddj['video'].find({'_id':ObjectId('568013c4d011361b96feb6a0')}).limit(1):
        print '====start ac solo', one['_id'], one['url']
        one['page_url'] = one['page_url'] + '#2931291'
        update_acfun(one)
        one = jxqctm_ddj['video'].find_one({'_id':one['_id']})
        print '====end', one['_id'], one['url']
    for one in jxqctm_ddj['video'].find({'_id':ObjectId("5680709dd011361b9600c049")}).limit(1):
        print '====start ac solo', one['_id'], one['url']
        one['page_url'] = one['page_url'] + '#2722991'
        update_acfun(one)
        one = jxqctm_ddj['video'].find_one({'_id':one['_id']})
        print '====end', one['_id'], one['url']

