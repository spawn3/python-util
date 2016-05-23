# -*- coding: UTF-8 -*-

import objcache


class ObjectCache(objcache.ObjectCache):

    def __init__(self):
        super(ObjectCache, self).__init__()


def cache_get_or_create(name, load_func, expire=600):
    cache_name = name + 'Cache'
    if not ObjectCache.exists(cache_name):
        ObjectCache.create(load_func, name=cache_name, expire=expire)
    return ObjectCache.get(cache_name)
