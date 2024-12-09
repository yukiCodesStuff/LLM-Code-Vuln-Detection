
    def tearDown(self):
        # wait on the server thread to terminate
        self.evt.wait(4.0)
        # XXX this code does not work, and in fact stop_serving doesn't exist.
        if not self.evt.is_set():
            self.evt.set()
            stop_serving()
            raise RuntimeError("timeout reached, test has failed")

        # disable traceback reporting
        xmlrpc.server.SimpleXMLRPCServer._send_traceback_header = False

        server = xmlrpclib.ServerProxy("http://%s:%d/RPC2" % (ADDR, PORT))
        self.assertEqual(server.add("a", "\xe9"), "a\xe9")

class MultiPathServerTestCase(BaseServerTestCase):
    threadFunc = staticmethod(http_multi_server)
    request_count = 2
    def test_path1(self):