# -*- coding: UTF-8 -*-

import redis

import objcache

redis_db = redis.Redis(host='localhost', port=6379, db=11)


class ObjectCache(objcache.ObjectCache):

    def __init__(self):
        super(ObjectCache, self).__init__()

    @staticmethod
    def exists(name):
        return name and name in ObjectCache._data


def cache_get_or_create(name, load_func, expire=600):
    cache_name = name + 'Cache'
    if not ObjectCache.exists(cache_name):
        ObjectCache.create(load_func, name=cache_name, expire=expire)
    return ObjectCache.get(cache_name)
