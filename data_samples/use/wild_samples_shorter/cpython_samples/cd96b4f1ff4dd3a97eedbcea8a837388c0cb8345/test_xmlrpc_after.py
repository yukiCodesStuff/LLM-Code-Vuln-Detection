
    def tearDown(self):
        # wait on the server thread to terminate
        self.evt.wait()

        # disable traceback reporting
        xmlrpc.server.SimpleXMLRPCServer._send_traceback_header = False

        server = xmlrpclib.ServerProxy("http://%s:%d/RPC2" % (ADDR, PORT))
        self.assertEqual(server.add("a", "\xe9"), "a\xe9")

    def test_partial_post(self):
        # Check that a partial POST doesn't make the server loop: issue #14001.
        conn = http.client.HTTPConnection(ADDR, PORT)
        conn.request('POST', '/RPC2 HTTP/1.0\r\nContent-Length: 100\r\n\r\nbye')
        conn.close()


class MultiPathServerTestCase(BaseServerTestCase):
    threadFunc = staticmethod(http_multi_server)
    request_count = 2
    def test_path1(self):