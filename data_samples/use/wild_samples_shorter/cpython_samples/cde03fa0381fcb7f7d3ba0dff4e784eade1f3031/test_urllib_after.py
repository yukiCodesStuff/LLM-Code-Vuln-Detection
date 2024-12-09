        self.assertTrue(urllib.request.proxy_bypass_environment('anotherdomain.com:8888'))
        self.assertTrue(urllib.request.proxy_bypass_environment('newdomain.com:1234'))

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

    def test_proxy_bypass_environment_host_match(self):
        bypass = urllib.request.proxy_bypass_environment
        self.env.set('NO_PROXY',
            'localhost, anotherdomain.com, newdomain.com:1234')