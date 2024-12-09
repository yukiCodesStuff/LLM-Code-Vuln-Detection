        bypass = {'exclude_simple': True, 'exceptions': []}
        self.assertTrue(_proxy_bypass_macosx_sysconf('test', bypass))

    def test_basic_auth(self, quote_char='"'):
        opener = OpenerDirector()
        password_manager = MockPasswordManager()
        auth_handler = urllib.request.HTTPBasicAuthHandler(password_manager)
        realm = "ACME Widget Store"
        http_handler = MockHTTPHandler(
            401, 'WWW-Authenticate: Basic realm=%s%s%s\r\n\r\n' %
            (quote_char, realm, quote_char))
        opener.add_handler(auth_handler)
        opener.add_handler(http_handler)
        self._test_basic_auth(opener, auth_handler, "Authorization",
                              realm, http_handler, password_manager,
                              "http://acme.example.com/protected",
                              "http://acme.example.com/protected",
                              )

    def test_basic_auth_with_single_quoted_realm(self):
        self.test_basic_auth(quote_char="'")

    def test_basic_auth_with_unquoted_realm(self):
        opener = OpenerDirector()
        password_manager = MockPasswordManager()
        auth_handler = urllib.request.HTTPBasicAuthHandler(password_manager)
        realm = "ACME Widget Store"
        http_handler = MockHTTPHandler(
            401, 'WWW-Authenticate: Basic realm=%s\r\n\r\n' % realm)
        opener.add_handler(auth_handler)
        opener.add_handler(http_handler)
        with self.assertWarns(UserWarning):
            self._test_basic_auth(opener, auth_handler, "Authorization",
                                realm, http_handler, password_manager,
                                "http://acme.example.com/protected",
                                "http://acme.example.com/protected",
                                )

    def test_proxy_basic_auth(self):
        opener = OpenerDirector()
        ph = urllib.request.ProxyHandler(dict(http="proxy.example.com:3128"))