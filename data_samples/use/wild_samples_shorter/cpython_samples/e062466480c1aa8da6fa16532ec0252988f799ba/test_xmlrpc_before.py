        # This avoids waiting for the socket timeout.
        self.test_simple1()

class MultiPathServerTestCase(BaseServerTestCase):
    threadFunc = staticmethod(http_multi_server)
    request_count = 2
    def test_path1(self):