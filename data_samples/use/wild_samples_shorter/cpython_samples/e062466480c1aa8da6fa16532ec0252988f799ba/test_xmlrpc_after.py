        # This avoids waiting for the socket timeout.
        self.test_simple1()

    def test_partial_post(self):
        # Check that a partial POST doesn't make the server loop: issue #14001.
        conn = httplib.HTTPConnection(ADDR, PORT)
        conn.request('POST', '/RPC2 HTTP/1.0\r\nContent-Length: 100\r\n\r\nbye')
        conn.close()

class MultiPathServerTestCase(BaseServerTestCase):
    threadFunc = staticmethod(http_multi_server)
    request_count = 2
    def test_path1(self):