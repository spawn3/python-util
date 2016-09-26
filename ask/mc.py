#!/usr/bin/env python


class Meta(type):
    def __init__(cls, *args, **kw):
        super(Meta, cls).__init__(*args, **kw)
        setattr(cls, '_name', 'value')



class A(Exception):
    __metaclass__ = Meta

    def __init__(self, **kw):
        if kw:
            name = '%s: %s' % (self._name, json.dumps(kw))
        else:
            name = self._name
        super(A, self).__init__(name)


if __name__ == '__main__':
    a = A()
    raise A
