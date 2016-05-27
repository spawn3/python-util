#!/usr/bin/env python
# -*- coding: utf-8 -*-


import tornado
import tornado.ioloop
import tornado.web
import tornado.httpclient
import tornado.gen
from tornado.httpserver import HTTPServer


class BaseHandler(tornado.web.RequestHandler):

    def prepare(self):
        pass

    def initialize(self):
        pass

    def get_current_user(self):
        pass

    def write_error(self, status_code, **kwargs):
        pass

    def send_error(self, status_code=500, **kwargs):
        pass

    def on_finish(self):
        pass


class MainHandler(BaseHandler):

    def get(self):
        # self.write("Hello, World")
        self.set_header('Content-Type', 'text/json')
        self.write({'code': 0, 'msg': 'hello'})


class AsyncHandler(BaseHandler):

    @tornado.web.asynchronous
    def get(self):
        url = 'http://so.picasso.adesk.com/emoji/v1/hits'
        http = tornado.httpclient.AsyncHTTPClient()
        http.fetch(url, callback=self.on_response)

    def on_response(self, response):
        if response.error:
            raise tornado.web.HTTPError(500)
        json = tornado.escape.json_decode(response.body)
        self.write(json)
        self.finish()


class GenHandler(BaseHandler):
    @tornado.gen.coroutine
    def get(self):
        url = 'http://so.picasso.adesk.com/emoji/v1/hits'
        http = tornado.httpclient.AsyncHTTPClient()
        response = yield http.fetch(url)
        json = tornado.escape.json_decode(response.body)
        self.write(json)


def make_app():
    """ global configuration, including the routing table that maps requests to handlers.

    :return:
    """
    return tornado.web.Application([
        (r'/', MainHandler),
        (r'/async', AsyncHandler),
        (r'/gen', GenHandler),
    ])


if __name__ == '__main__':
    print 'Listening 8888...'
    app = make_app()
    app.listen(8888)
    tornado.ioloop.IOLoop.current().start()
