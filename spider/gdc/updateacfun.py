#!/usr/bin/python
# coding=utf-8

import pymongo

from webcrawl.handleRequest import requGet

if __name__ == '__main__':
    mc = pymongo.MongoClient('localhost')
    ddj = mc['dandan-jiang']
    for one in ddj.video.find({'src':'Acfun'}):
        if one['parent_page_id'] is None:
            if not '#' in one['page_url']:
                print '---solo---'
                continue
                # try:
                #     info_result = requGet(one['page_url'], timeout=30, format='JSON')
                #     page_url = one['page_url'] + '#' + str(info_result['data']['fullContent']['videos'][0]['videoId'])
                # except:
                #     print 'error url', one['page_url']
                #     continue
                # if ddj.video.find_one({'page_url':page_url}):
                #     print '===remove solo', one['_id']
                #     ddj.video.remove({'_id':one['_id']})
                #     ddj.video_rubbish.insert(one)
                # else:
                #     print '===solo', page_url
                #     ddj.video.update({'_id':one['_id']}, {'$set':{'page_url':page_url, 'page_id':hash(page_url)}})
            else:
                page_url = one['page_url'][:one['page_url'].rindex('#')]
                exist = ddj.video.find_one({'page_url':page_url})
                if exist is not None:
                    ddj.video.remove({'_id':exist['_id']})
                    ddj.video_rubbish.insert(exist)
        else:
            if not '#' in one['page_url']:
                print '---album---'
                continue
                # url = 'http://www.acfun.tv/bangumi/bangumi/info?id=%s' % one['page_url'][one['page_url'].rindex('/')+1:one['page_url'].rindex('_')].replace('ab', '')
                # info_result = requGet(url, timeout=30, format='JSON')
                # pages = info_result['data']['videos']
                # pages = sorted(pages, key=lambda v:v['sort'])
                # albums = list(ddj.video.find({'parent_page_id':one['parent_page_id']}).sort([('snum', 1), ]))
                # album_urls = [str(one['page_url'][:one['page_url'].rindex('#')+1]) for one in albums if '#' in one['page_url']]
                # print album_urls
                # for album_one in albums:
                #     if not '#' in album_one['page_url']:
                #         if '%s%s' % (album_one['page_url'], '#') in album_urls:
                #             print '===remove album', album_one['_id']
                #             ddj.video.remove({'_id':album_one['_id']})
                #             ddj.video_rubbish.insert(one)
                #         else:
                #             page_url = album_one['page_url'] + '#' + pages[album_one['snum'] - 1]['danmakuId']
                #             print '===album', page_url
                #             ddj.video.update({'_id':one['_id']}, {'$set':{'page_url':page_url, 'page_id':hash(page_url)}})
            else:
                page_url = one['page_url'][:one['page_url'].rindex('#')]
                exist = ddj.video.find_one({'page_url':page_url})
                if exist is not None:
                    ddj.video.remove({'_id':exist['_id']})
                    ddj.video_rubbish.insert(exist)

