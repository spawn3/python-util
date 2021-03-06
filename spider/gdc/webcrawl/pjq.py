#!/usr/bin/python
# coding=utf-8
import json
import heapq
import redis
import beanstalkc
import threading
import queue
threading.queue = queue
import cPickle as pickle
from bson import ObjectId

from character import unicode2utf8

try:
    from kokolog.aboutfile import modulename, modulepath
    from kokolog.prettyprint import logprint
except:
    def modulename(n):
        return None

    def modulepath(p):
        return None

    def logprint(n, p):
        def _wraper(*args, **kwargs):
            print(' '.join(args))
        return _wraper, None

_print, logger = logprint(modulename(__file__), modulepath(__file__))

DESCRIBE = {0:'ERROR', 1:'COMPLETED', 2:'WAIT', 'READY':10, 3:'RUNNING', 4:'RETRY', 5:'ABANDONED'}

class RedisQueue(object):
    conditions = {}

    def __init__(self, host='localhost', port=6379, db=0, tube='default', timeout=30, items=None, unfinished_tasks=None, init=True, weight=[]):
        self.rc = redis.StrictRedis(host='localhost', port=port, db=db)
        self.tube = tube
        self.unfinished_tasks = 0

        if self.tube in RedisQueue.conditions:
            pass
        else:
            RedisQueue.conditions[self.tube] = {'unfinished_tasks': unfinished_tasks or 0, 'event': threading.Event(), 'mutex':threading.Lock(), 'weight':weight}
            RedisQueue.conditions[self.tube]['event'].set()
        if init:
            self.clear()
        if items:
            for item in items:
                self.put(item)

    def sid(self):
        return str(ObjectId())

    def put(self, item):
        priority, methodId, methodName, times, args, kwargs, tid = item
        # self.rc.zadd(self.tube, pickle.dumps({'priority': priority, 'methodId': methodId,
        #                         'times': times, 'args': args, 'kwargs': kwargs}), priority)
        sid = self.sid()
        self.rc.lpush('-'.join([str(self.tube), str(priority)]), pickle.dumps({'priority': priority, 'methodId': methodId, 'methodName':methodName, 'times': times, 'args': args, 'kwargs': kwargs, 'tid':tid, 'sid':sid}))
        if times == 0:
            self.rc.hset('pholcus-state', sid, 2)
        else:
            self.rc.hset('pholcus-state', sid, 4)
        RedisQueue.conditions[self.tube]['unfinished_tasks'] += 1
        RedisQueue.conditions[self.tube]['event'].clear()

    def get(self, block=True, timeout=0):
        # item = self.rc.zrangebyscore(self.tube, float('-inf'), float('+inf'), start=0, num=1)
        item = self.rc.brpop(['-'.join([str(self.tube), str(one)]) for one in RedisQueue.conditions[self.tube]['weight']], timeout=timeout)
        if item:
            item = item[-1]
            item = pickle.loads(item)
            if self.rc.hget('pholcus-state', item['sid']) == 5:
                self.rc.hdel('pholcus-state', item['sid'])
                _print('', tid=item['tid'], sid=item['sid'], type='ABANDONED', status=2, sname='', priority=item['priority'], times=item['times'], args='(%s)' % ', '.join([str(one) for one in item['args']]), kwargs=json.dumps(item['kwargs'], ensure_ascii=False), txt=None)
                return None
            else:
                self.rc.hset('pholcus-state', item['sid'], 3)
                return (item['priority'], item['methodId'], item['methodName'], item['times'], tuple(item['args']), item['kwargs'], item['tid']), item['sid']
        else:
            return None

    def empty(self):
        total = sum([self.rc.llen(one) for one in ['-'.join([str(self.tube), str(one)]) for one in RedisQueue.conditions[self.tube]['weight']]])
        return total == 0

    def copy(self):
        pass

    def task_done(self, item, force=False):
        if item is not None:
            tid, sname, priority, times, args, kwargs, sid = item
            _print('', tid=tid, sid=sid, type='COMPLETED', status=1, sname=sname, priority=priority, times=times, args='(%s)' % ', '.join([str(one) for one in args]), kwargs=json.dumps(kwargs, ensure_ascii=False), txt=None)
            self.rc.hdel('pholcus-state', sid)
        if RedisQueue.conditions[self.tube]['unfinished_tasks'] <= 0:
            # raise ValueError('task_done() called too many times')
            pass
        RedisQueue.conditions[self.tube]['unfinished_tasks'] -= 1
        if RedisQueue.conditions[self.tube]['unfinished_tasks'] == 0 or force:
            # if self.empty() or force:
            RedisQueue.conditions[self.tube]['event'].set()

    def join(self):
        RedisQueue.conditions[self.tube]['event'].wait()

    def clear(self):
        for one in self.rc.keys():
            if one.startswith(self.tube):
                self.rc.delete(one)

    def rank(self, weight):
        RedisQueue.conditions[self.tube]['mutex'].acquire()
        RedisQueue.conditions[self.tube]['weight'].extend(weight)
        RedisQueue.conditions[self.tube]['weight'].sort()
        RedisQueue.conditions[self.tube]['mutex'].release()

    def total(self):
        total = 0
        for one in self.rc.keys():
            if one.startswith(self.tube):
                total += self.rc.llen(one)
        return total

    def abandon(self, sid):
        self.rc.hset('pholcus-state', sid, 5)

    def traversal(self, skip=0, limit=10):
        tubes = [one for one in self.rc.keys() if one.startswith(self.tube)]
        tubes.sort()
        result = []
        start = skip
        end = skip + limit - 1
        flag = False
        for tube in tubes:
            for item in self.rc.lrange(tube, start, end):
                item = pickle.loads(item)
                item['status_num'] = self.rc.hget('pholcus-state', item['sid']) or 3
                if len(result) + skip > DESCRIBE['READY']:
                    item['status_desc'] = DESCRIBE.get(int(item['status_num']))
                else:
                    item['status_desc'] = 'ready'
                result.append(item)
                if len(result) == limit:
                    flag = True
                    break
            else:
                start = 0
                end = limit - len(result) - 1
            if flag:
                break
        return result

    def __repr__(self):
        return "<" + str(self.__class__).replace(" ", "").replace("'", "").split('.')[-1]

    def collect(self):
        if self.tube in RedisQueue.conditions:
            del RedisQueue.conditions[self.tube]

    def __del__(self):
        del self.rc


class BeanstalkdQueue(object):
    conditions = {}

    def __init__(self, host='localhost', port=11300, tube='default', timeout=30, items=None, unfinished_tasks=None):
        self.bc = beanstalkc.Connection(host, port, connect_timeout=timeout)
        self.tube = tube
        self.bc.use(self.tube)
        self.bc.watch(self.tube)
        if self.tube in BeanstalkdQueue.conditions:
            pass
        else:
            BeanstalkdQueue.conditions[self.tube] = {'unfinished_tasks': unfinished_tasks or 0, 'event': threading.Event()}
            self.clear()
            BeanstalkdQueue.conditions[self.tube]['event'].set()
        if items:
            for item in items:
                self.put(item)

    def put(self, item):
        priority, methodId, methodName, times, args, kwargs, tid = item
        self.bc.put(pickle.dumps({'priority': priority, 'methodId': methodId, 'methodName':methodName,
                                'times': times, 'args': args, 'kwargs': kwargs, 'tid':tid}), priority=priority)
        BeanstalkdQueue.conditions[self.tube]['unfinished_tasks'] += 1
        BeanstalkdQueue.conditions[self.tube]['event'].clear()

    def get(self, block=True, timeout=0):
        item = self.bc.reserve(timeout=timeout)
        if item:
            item.delete()
            item = pickle.loads(item.body)
            return (item['priority'], item['methodId'], item['methodName'], item['times'], tuple(item['args']), item['kwargs'], item['tid']), None
        else:
            return None

    def empty(self):
        return self.bc.stats_tube(self.tube)['current-jobs-ready'] == 0

    def copy(self):
        pass

    def task_done(self, item, force=False):
        if BeanstalkdQueue.conditions[self.tube]['unfinished_tasks'] <= 0:
            raise ValueError('task_done() called too many times')
        BeanstalkdQueue.conditions[self.tube]['unfinished_tasks'] -= 1
        if BeanstalkdQueue.conditions[self.tube]['unfinished_tasks'] == 0 or force:
            # if self.empty() or force:
            BeanstalkdQueue.conditions[self.tube]['event'].set()

    def join(self):
        BeanstalkdQueue.conditions[self.tube]['event'].wait()

    def clear(self):
        while not self.empty():
            item = self.get(timeout=10)
            del item

    def rank(self, weight):
        pass

    def __repr__(self):
        return "<" + str(self.__class__).replace(" ", "").replace("'", "").split('.')[-1]

    def collect(self):
        if self.tube in BeanstalkdQueue.conditions:
            del BeanstalkdQueue.conditions[self.tube]

    def __del__(self):
        del self.bc



# class LocalQueue(threading.queue.Queue):

#     def __new__(cls):
#         return threading.queue.Queue.__new__(cls)
def LocalQueue():

    def __init__(self, maxsize=None, items=None, unfinished_tasks=None):
        self.is_patch = not 'join' in dir(threading.queue.Queue)
        self.maxsize = maxsize or 0
        self.items = items

        self.parent = threading.queue.Queue.__init__(self, maxsize)
        if self.is_patch:
            from gevent.event import Event
            self._cond = Event()
            self._cond.set()

        if unfinished_tasks:
            self.unfinished_tasks = unfinished_tasks
        elif items:
            self.unfinished_tasks = len(items)
        else:
            self.unfinished_tasks = 0

        if self.is_patch and self.unfinished_tasks:
            self._cond.clear()

    def _init(self, maxsize):
        if self.items:
            self.queue = list(items)
        else:
            self.queue = []

    def _put(self, item, heappush=heapq.heappush):
        heappush(self.queue, item)
        if self.is_patch:
            self.unfinished_tasks += 1
            self._cond.clear()

    def _get(self, heappop=heapq.heappop):
        return heappop(self.queue), None

    def task_done(self, item, force=False):
        if self.is_patch:
            if self.unfinished_tasks <= 0:
                raise ValueError('task_done() called too many times')
            self.unfinished_tasks -= 1
            if self.unfinished_tasks == 0 or force:
                self._cond.set()
        else:
            self.all_tasks_done.acquire()
            try:
                unfinished = self.unfinished_tasks - 1
                if unfinished <= 0 or force:
                    if unfinished < 0:
                        raise ValueError('task_done() called too many times')
                    self.all_tasks_done.notify_all()
                self.unfinished_tasks = unfinished
            finally:
                self.all_tasks_done.release()

    def join(self):
        if self.is_patch:
            self._cond.wait()
        else:
            # self.parent.join()
            self.all_tasks_done.acquire()
            try:
                while self.unfinished_tasks:
                    self.all_tasks_done.wait()
            finally:
                self.all_tasks_done.release()

    def rank(self, weight):
        pass

    def collect(self):
        pass

    return type('PriorityQueue', (threading.queue.Queue, ), {'__init__':__init__, '_init':_init, '_put':_put, '_get':_get, 'task_done':task_done, 'join':join, 'rank':rank, 'collect':collect})


if __name__ == '__main__':
    from gevent.queue import JoinableQueue
    import gevent
    q = LocalQueue()

    def worker():
        while True:
            item = q.get()
            try:
                gevent.sleep(5)
            finally:
                q.task_done()

    def consume():
        for i in range(10):
            gevent.spawn(worker)

    def produce():
        for item in range(20):
            q.put((20 - item, item))
            gevent.sleep(0.1)

    import threading
    consume()
    produce()
    # a = threading.Thread(target=consume)
    # b = threading.Thread(target=produce)
    # a.start()
    # b.start()

    q.join()

