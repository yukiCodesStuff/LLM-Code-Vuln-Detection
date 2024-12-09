        self.env.set('NO_PROXY', 'localhost, anotherdomain.com, newdomain.com')
        self.assertTrue(urllib.request.proxy_bypass_environment('anotherdomain.com'))

class urlopen_HttpTests(unittest.TestCase, FakeHTTPMixin):
    """Test urlopen() opening a fake http connection."""

    def check_read(self, ver):