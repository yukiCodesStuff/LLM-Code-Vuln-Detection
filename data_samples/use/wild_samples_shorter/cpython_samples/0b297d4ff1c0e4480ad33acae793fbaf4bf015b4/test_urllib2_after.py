        bypass = {'exclude_simple': True, 'exceptions': []}
        self.assertTrue(_proxy_bypass_macosx_sysconf('test', bypass))

    def check_basic_auth(self, headers, realm):
        with self.subTest(realm=realm, headers=headers):
            opener = OpenerDirector()
            password_manager = MockPasswordManager()
            auth_handler = urllib.request.HTTPBasicAuthHandler(password_manager)
            body = '\r\n'.join(headers) + '\r\n\r\n'
            http_handler = MockHTTPHandler(401, body)
            opener.add_handler(auth_handler)
            opener.add_handler(http_handler)
            self._test_basic_auth(opener, auth_handler, "Authorization",
                                  realm, http_handler, password_manager,
                                  "http://acme.example.com/protected",
                                  "http://acme.example.com/protected")

    def test_basic_auth(self):
        realm = "realm2@example.com"
        realm2 = "realm2@example.com"
        basic = f'Basic realm="{realm}"'
        basic2 = f'Basic realm="{realm2}"'
        other_no_realm = 'Otherscheme xxx'
        digest = (f'Digest realm="{realm2}", '
                  f'qop="auth, auth-int", '
                  f'nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093", '
                  f'opaque="5ccc069c403ebaf9f0171e9517f40e41"')
        for realm_str in (
            # test "quote" and 'quote'
            f'Basic realm="{realm}"',
            f"Basic realm='{realm}'",

            # charset is ignored
            f'Basic realm="{realm}", charset="UTF-8"',

            # Multiple challenges per header
            f'{basic}, {basic2}',
            f'{basic}, {other_no_realm}',
            f'{other_no_realm}, {basic}',
            f'{basic}, {digest}',
            f'{digest}, {basic}',
        ):
            headers = [f'WWW-Authenticate: {realm_str}']
            self.check_basic_auth(headers, realm)

        # no quote: expect a warning
        with support.check_warnings(("Basic Auth Realm was unquoted",
                                     UserWarning)):
            headers = [f'WWW-Authenticate: Basic realm={realm}']
            self.check_basic_auth(headers, realm)

        # Multiple headers: one challenge per header.
        # Use the first Basic realm.
        for challenges in (
            [basic,  basic2],
            [basic,  digest],
            [digest, basic],
        ):
            headers = [f'WWW-Authenticate: {challenge}'
                       for challenge in challenges]
            self.check_basic_auth(headers, realm)

    def test_proxy_basic_auth(self):
        opener = OpenerDirector()
        ph = urllib.request.ProxyHandler(dict(http="proxy.example.com:3128"))