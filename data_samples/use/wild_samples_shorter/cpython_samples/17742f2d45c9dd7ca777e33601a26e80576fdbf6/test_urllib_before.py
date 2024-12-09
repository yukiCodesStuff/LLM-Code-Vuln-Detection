        self.assertTrue(urllib.request.proxy_bypass_environment('anotherdomain.com:8888'))
        self.assertTrue(urllib.request.proxy_bypass_environment('newdomain.com:1234'))

    def test_proxy_bypass_environment_host_match(self):
        bypass = urllib.request.proxy_bypass_environment
        self.env.set('NO_PROXY',
            'localhost, anotherdomain.com, newdomain.com:1234')