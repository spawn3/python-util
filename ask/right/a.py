#!/usr/bin/env python


from pprint import pprint
from base import register, register2


__all__ = ['A', 'B']


#@register(dct=__dict__)
@register2(name=__name__)
class A(object):
    pass


#@register(dct=globals())
@register2(name=__name__)
class B(object):
    pass


# pprint(globals())
