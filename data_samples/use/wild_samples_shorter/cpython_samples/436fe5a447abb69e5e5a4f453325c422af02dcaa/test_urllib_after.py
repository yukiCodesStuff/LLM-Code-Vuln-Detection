        self.env.set('NO_PROXY', 'localhost, anotherdomain.com, newdomain.com')
        self.assertTrue(urllib.request.proxy_bypass_environment('anotherdomain.com'))

    def test_proxy_cgi_ignore(self):
        try:
            self.env.set('HTTP_PROXY', 'http://somewhere:3128')
            proxies = urllib.request.getproxies_environment()
            self.assertEqual('http://somewhere:3128', proxies['http'])
            self.env.set('REQUEST_METHOD', 'GET')
            proxies = urllib.request.getproxies_environment()
            self.assertNotIn('http', proxies)
        finally:
            self.env.unset('REQUEST_METHOD')
            self.env.unset('HTTP_PROXY')


class urlopen_HttpTests(unittest.TestCase, FakeHTTPMixin, FakeFTPMixin):
    """Test urlopen() opening a fake http connection."""

    def check_read(self, ver):