#-*- conding:utf-8-*-

import time
import uuid
import inspect


class DataObject(object):
    '''
    object cahced by app
    '''
    
    def __init__(self, key, data_source, expire=600):
        self.key = key
        self.expire = expire
        self.data_source = data_source
        self.last_visit_atime = time.time()
        self._value = None

    def is_expired(self):
        return time.time() - self.last_visit_atime > self.expire

    def update_visit_time(self):
        self.last_visit_atime = time.time()

    @property
    def value(self):
        if self._value is None or self.is_expired():
            self._value = self.data_source()
            self.update_visit_time()
        return self._value

    def __repr__(self):
        return '<key: %s, value: %s, expire: %d, fn: %s>' % (self.key, self.value, self.expire, self.data_source)


class ObjectCache(object):
    """App local cache object 

    Attribute:
        _data: cache all data
    """
    _data = {}

    @staticmethod
    def add(data_source, name=None, expire=600):
        """
        args:
            data_source: load data, a callable object
            name: given by developer to mark the cache data 
            expire: cache expire time

        return:
            obj_key: the key that finally is used to mark the cache data 
        """
        if not callable(data_source):
            raise Exception("data_source %s must be callable " % data_source)
        
        if not isinstance(expire, int):
            raise Exception("expire %s must be int type" % expire)

        if name:
            try:
                hash(name)
            except TypeError:
                raise TypeError("name %s is unhashable"% name)
            obj_key = name
        else:
            obj_key = uuid.uuid4()

        ObjectCache._data[obj_key] = DataObject(obj_key, data_source, expire)
        return obj_key

    @staticmethod
    def delete(obj_key):
        try:
            del ObjectCache._data[obj_key]
        except:
            pass

    @staticmethod
    def get(obj_key):
        if obj_key not in ObjectCache._data:
            raise Exception("%s is invalid key" % obj_key)

        obj_data = ObjectCache._data.get(obj_key)
        if isinstance(obj_data, DataObject):
            return obj_data.value
        return None

    @staticmethod
    def exists(name):
        return name and name in ObjectCache._data

    @staticmethod
    def scan():
        for k, v in ObjectCache._data.iteritems():
            print k, v


if __name__ == '__main__':
    def data_source():
        return [1, 2, 3]

    ObjectCache.add(data_source, 'name1')
    print ObjectCache.get('name1')
    ObjectCache.scan()
    ObjectCache.delete('name1')
    ObjectCache.delete('name2')
    ObjectCache.scan()
