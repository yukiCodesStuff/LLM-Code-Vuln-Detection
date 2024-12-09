                'notinbypass'):
            self.assertFalse(_proxy_bypass_macosx_sysconf(host, bypass),
                             'expected bypass of %s to be False' % host)

        # Check the exclude_simple flag
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
        opener.add_handler(ph)
        password_manager = MockPasswordManager()
        auth_handler = urllib.request.ProxyBasicAuthHandler(password_manager)
        realm = "ACME Networks"
        http_handler = MockHTTPHandler(
            407, 'Proxy-Authenticate: Basic realm="%s"\r\n\r\n' % realm)
        opener.add_handler(auth_handler)
        opener.add_handler(http_handler)
        self._test_basic_auth(opener, auth_handler, "Proxy-authorization",
                              realm, http_handler, password_manager,
                              "http://acme.example.com:3128/protected",
                              "proxy.example.com:3128",
                              )

    def test_basic_and_digest_auth_handlers(self):
        # HTTPDigestAuthHandler raised an exception if it couldn't handle a 40*
        # response (http://python.org/sf/1479302), where it should instead
        # return None to allow another handler (especially
        # HTTPBasicAuthHandler) to handle the response.

        # Also (http://python.org/sf/14797027, RFC 2617 section 1.2), we must
        # try digest first (since it's the strongest auth scheme), so we record
        # order of calls here to check digest comes first:
        class RecordingOpenerDirector(OpenerDirector):
            def __init__(self):
                OpenerDirector.__init__(self)
                self.recorded = []

            def record(self, info):
                self.recorded.append(info)

        class TestDigestAuthHandler(urllib.request.HTTPDigestAuthHandler):
            def http_error_401(self, *args, **kwds):
                self.parent.record("digest")
                urllib.request.HTTPDigestAuthHandler.http_error_401(self,
                                                             *args, **kwds)

        class TestBasicAuthHandler(urllib.request.HTTPBasicAuthHandler):
            def http_error_401(self, *args, **kwds):
                self.parent.record("basic")
                urllib.request.HTTPBasicAuthHandler.http_error_401(self,
                                                            *args, **kwds)

        opener = RecordingOpenerDirector()
        password_manager = MockPasswordManager()
        digest_handler = TestDigestAuthHandler(password_manager)
        basic_handler = TestBasicAuthHandler(password_manager)
        realm = "ACME Networks"
        http_handler = MockHTTPHandler(
            401, 'WWW-Authenticate: Basic realm="%s"\r\n\r\n' % realm)
        opener.add_handler(basic_handler)
        opener.add_handler(digest_handler)
        opener.add_handler(http_handler)

        # check basic auth isn't blocked by digest handler failing
        self._test_basic_auth(opener, basic_handler, "Authorization",
                              realm, http_handler, password_manager,
                              "http://acme.example.com/protected",
                              "http://acme.example.com/protected",
                              )
        # check digest was tried before basic (twice, because
        # _test_basic_auth called .open() twice)
        self.assertEqual(opener.recorded, ["digest", "basic"]*2)

    def test_unsupported_auth_digest_handler(self):
        opener = OpenerDirector()
        # While using DigestAuthHandler
        digest_auth_handler = urllib.request.HTTPDigestAuthHandler(None)
        http_handler = MockHTTPHandler(
            401, 'WWW-Authenticate: Kerberos\r\n\r\n')
        opener.add_handler(digest_auth_handler)
        opener.add_handler(http_handler)
        self.assertRaises(ValueError, opener.open, "http://www.example.com")

    def test_unsupported_auth_basic_handler(self):
        # While using BasicAuthHandler
        opener = OpenerDirector()
        basic_auth_handler = urllib.request.HTTPBasicAuthHandler(None)
        http_handler = MockHTTPHandler(
            401, 'WWW-Authenticate: NTLM\r\n\r\n')
        opener.add_handler(basic_auth_handler)
        opener.add_handler(http_handler)
        self.assertRaises(ValueError, opener.open, "http://www.example.com")

    def _test_basic_auth(self, opener, auth_handler, auth_header,
                         realm, http_handler, password_manager,
                         request_url, protected_url):
        import base64
        user, password = "wile", "coyote"

        # .add_password() fed through to password manager
        auth_handler.add_password(realm, request_url, user, password)
        self.assertEqual(realm, password_manager.realm)
        self.assertEqual(request_url, password_manager.url)
        self.assertEqual(user, password_manager.user)
        self.assertEqual(password, password_manager.password)

        opener.open(request_url)

        # should have asked the password manager for the username/password
        self.assertEqual(password_manager.target_realm, realm)
        self.assertEqual(password_manager.target_url, protected_url)

        # expect one request without authorization, then one with
        self.assertEqual(len(http_handler.requests), 2)
        self.assertFalse(http_handler.requests[0].has_header(auth_header))
        userpass = bytes('%s:%s' % (user, password), "ascii")
        auth_hdr_value = ('Basic ' +
            base64.encodebytes(userpass).strip().decode())
        self.assertEqual(http_handler.requests[1].get_header(auth_header),
                         auth_hdr_value)
        self.assertEqual(http_handler.requests[1].unredirected_hdrs[auth_header],
                         auth_hdr_value)
        # if the password manager can't find a password, the handler won't
        # handle the HTTP auth error
        password_manager.user = password_manager.password = None
        http_handler.reset()
        opener.open(request_url)
        self.assertEqual(len(http_handler.requests), 1)
        self.assertFalse(http_handler.requests[0].has_header(auth_header))

    def test_basic_prior_auth_auto_send(self):
        # Assume already authenticated if is_authenticated=True
        # for APIs like Github that don't return 401

        user, password = "wile", "coyote"
        request_url = "http://acme.example.com/protected"

        http_handler = MockHTTPHandlerCheckAuth(200)

        pwd_manager = HTTPPasswordMgrWithPriorAuth()
        auth_prior_handler = HTTPBasicAuthHandler(pwd_manager)
        auth_prior_handler.add_password(
            None, request_url, user, password, is_authenticated=True)

        is_auth = pwd_manager.is_authenticated(request_url)
        self.assertTrue(is_auth)

        opener = OpenerDirector()
        opener.add_handler(auth_prior_handler)
        opener.add_handler(http_handler)

        opener.open(request_url)

        # expect request to be sent with auth header
        self.assertTrue(http_handler.has_auth_header)

    def test_basic_prior_auth_send_after_first_success(self):
        # Auto send auth header after authentication is successful once

        user, password = 'wile', 'coyote'
        request_url = 'http://acme.example.com/protected'
        realm = 'ACME'

        pwd_manager = HTTPPasswordMgrWithPriorAuth()
        auth_prior_handler = HTTPBasicAuthHandler(pwd_manager)
        auth_prior_handler.add_password(realm, request_url, user, password)

        is_auth = pwd_manager.is_authenticated(request_url)
        self.assertFalse(is_auth)

        opener = OpenerDirector()
        opener.add_handler(auth_prior_handler)

        http_handler = MockHTTPHandler(
            401, 'WWW-Authenticate: Basic realm="%s"\r\n\r\n' % None)
        opener.add_handler(http_handler)

        opener.open(request_url)

        is_auth = pwd_manager.is_authenticated(request_url)
        self.assertTrue(is_auth)

        http_handler = MockHTTPHandlerCheckAuth(200)
        self.assertFalse(http_handler.has_auth_header)

        opener = OpenerDirector()
        opener.add_handler(auth_prior_handler)
        opener.add_handler(http_handler)

        # After getting 200 from MockHTTPHandler
        # Next request sends header in the first request
        opener.open(request_url)

        # expect request to be sent with auth header
        self.assertTrue(http_handler.has_auth_header)

    def test_http_closed(self):
        """Test the connection is cleaned up when the response is closed"""
        for (transfer, data) in (
            ("Connection: close", b"data"),
            ("Transfer-Encoding: chunked", b"4\r\ndata\r\n0\r\n\r\n"),
            ("Content-Length: 4", b"data"),
        ):
            header = "HTTP/1.1 200 OK\r\n{}\r\n\r\n".format(transfer)
            conn = test_urllib.fakehttp(header.encode() + data)
            handler = urllib.request.AbstractHTTPHandler()
            req = Request("http://dummy/")
            req.timeout = None
            with handler.do_open(conn, req) as resp:
                resp.read()
            self.assertTrue(conn.fakesock.closed,
                "Connection not closed with {!r}".format(transfer))

    def test_invalid_closed(self):
        """Test the connection is cleaned up after an invalid response"""
        conn = test_urllib.fakehttp(b"")
        handler = urllib.request.AbstractHTTPHandler()
        req = Request("http://dummy/")
        req.timeout = None
        with self.assertRaises(http.client.BadStatusLine):
            handler.do_open(conn, req)
        self.assertTrue(conn.fakesock.closed, "Connection not closed")


class MiscTests(unittest.TestCase):

    def opener_has_handler(self, opener, handler_class):
        self.assertTrue(any(h.__class__ == handler_class
                            for h in opener.handlers))

    def test_build_opener(self):
        class MyHTTPHandler(urllib.request.HTTPHandler):
            pass

        class FooHandler(urllib.request.BaseHandler):
            def foo_open(self):
                pass

        class BarHandler(urllib.request.BaseHandler):
            def bar_open(self):
                pass

        build_opener = urllib.request.build_opener

        o = build_opener(FooHandler, BarHandler)
        self.opener_has_handler(o, FooHandler)
        self.opener_has_handler(o, BarHandler)

        # can take a mix of classes and instances
        o = build_opener(FooHandler, BarHandler())
        self.opener_has_handler(o, FooHandler)
        self.opener_has_handler(o, BarHandler)

        # subclasses of default handlers override default handlers
        o = build_opener(MyHTTPHandler)
        self.opener_has_handler(o, MyHTTPHandler)

        # a particular case of overriding: default handlers can be passed
        # in explicitly
        o = build_opener()
        self.opener_has_handler(o, urllib.request.HTTPHandler)
        o = build_opener(urllib.request.HTTPHandler)
        self.opener_has_handler(o, urllib.request.HTTPHandler)
        o = build_opener(urllib.request.HTTPHandler())
        self.opener_has_handler(o, urllib.request.HTTPHandler)

        # Issue2670: multiple handlers sharing the same base class
        class MyOtherHTTPHandler(urllib.request.HTTPHandler):
            pass

        o = build_opener(MyHTTPHandler, MyOtherHTTPHandler)
        self.opener_has_handler(o, MyHTTPHandler)
        self.opener_has_handler(o, MyOtherHTTPHandler)

    @unittest.skipUnless(support.is_resource_enabled('network'),
                         'test requires network access')
    def test_issue16464(self):
        with support.transient_internet("http://www.example.com/"):
            opener = urllib.request.build_opener()
            request = urllib.request.Request("http://www.example.com/")
            self.assertEqual(None, request.data)

            opener.open(request, "1".encode("us-ascii"))
            self.assertEqual(b"1", request.data)
            self.assertEqual("1", request.get_header("Content-length"))

            opener.open(request, "1234567890".encode("us-ascii"))
            self.assertEqual(b"1234567890", request.data)
            self.assertEqual("10", request.get_header("Content-length"))

    def test_HTTPError_interface(self):
        """
        Issue 13211 reveals that HTTPError didn't implement the URLError
        interface even though HTTPError is a subclass of URLError.
        """
        msg = 'something bad happened'
        url = code = fp = None
        hdrs = 'Content-Length: 42'
        err = urllib.error.HTTPError(url, code, msg, hdrs, fp)
        self.assertTrue(hasattr(err, 'reason'))
        self.assertEqual(err.reason, 'something bad happened')
        self.assertTrue(hasattr(err, 'headers'))
        self.assertEqual(err.headers, 'Content-Length: 42')
        expected_errmsg = 'HTTP Error %s: %s' % (err.code, err.msg)
        self.assertEqual(str(err), expected_errmsg)
        expected_errmsg = '<HTTPError %s: %r>' % (err.code, err.msg)
        self.assertEqual(repr(err), expected_errmsg)

    def test_parse_proxy(self):
        parse_proxy_test_cases = [
            ('proxy.example.com',
             (None, None, None, 'proxy.example.com')),
            ('proxy.example.com:3128',
             (None, None, None, 'proxy.example.com:3128')),
            ('proxy.example.com', (None, None, None, 'proxy.example.com')),
            ('proxy.example.com:3128',
             (None, None, None, 'proxy.example.com:3128')),
            # The authority component may optionally include userinfo
            # (assumed to be # username:password):
            ('joe:password@proxy.example.com',
             (None, 'joe', 'password', 'proxy.example.com')),
            ('joe:password@proxy.example.com:3128',
             (None, 'joe', 'password', 'proxy.example.com:3128')),
            #Examples with URLS
            ('http://proxy.example.com/',
             ('http', None, None, 'proxy.example.com')),
            ('http://proxy.example.com:3128/',
             ('http', None, None, 'proxy.example.com:3128')),
            ('http://joe:password@proxy.example.com/',
             ('http', 'joe', 'password', 'proxy.example.com')),
            ('http://joe:password@proxy.example.com:3128',
             ('http', 'joe', 'password', 'proxy.example.com:3128')),
            # Everything after the authority is ignored
            ('ftp://joe:password@proxy.example.com/rubbish:3128',
             ('ftp', 'joe', 'password', 'proxy.example.com')),
            # Test for no trailing '/' case
            ('http://joe:password@proxy.example.com',
             ('http', 'joe', 'password', 'proxy.example.com'))
        ]

        for tc, expected in parse_proxy_test_cases:
            self.assertEqual(_parse_proxy(tc), expected)

        self.assertRaises(ValueError, _parse_proxy, 'file:/ftp.example.com'),

    def test_unsupported_algorithm(self):
        handler = AbstractDigestAuthHandler()
        with self.assertRaises(ValueError) as exc:
            handler.get_algorithm_impls('invalid')
        self.assertEqual(
            str(exc.exception),
            "Unsupported digest authentication algorithm 'invalid'"
        )


class RequestTests(unittest.TestCase):
    class PutRequest(Request):
        method = 'PUT'

    def setUp(self):
        self.get = Request("http://www.python.org/~jeremy/")
        self.post = Request("http://www.python.org/~jeremy/",
                            "data",
                            headers={"X-Test": "test"})
        self.head = Request("http://www.python.org/~jeremy/", method='HEAD')
        self.put = self.PutRequest("http://www.python.org/~jeremy/")
        self.force_post = self.PutRequest("http://www.python.org/~jeremy/",
            method="POST")

    def test_method(self):
        self.assertEqual("POST", self.post.get_method())
        self.assertEqual("GET", self.get.get_method())
        self.assertEqual("HEAD", self.head.get_method())
        self.assertEqual("PUT", self.put.get_method())
        self.assertEqual("POST", self.force_post.get_method())

    def test_data(self):
        self.assertFalse(self.get.data)
        self.assertEqual("GET", self.get.get_method())
        self.get.data = "spam"
        self.assertTrue(self.get.data)
        self.assertEqual("POST", self.get.get_method())

    # issue 16464
    # if we change data we need to remove content-length header
    # (cause it's most probably calculated for previous value)
    def test_setting_data_should_remove_content_length(self):
        self.assertNotIn("Content-length", self.get.unredirected_hdrs)
        self.get.add_unredirected_header("Content-length", 42)
        self.assertEqual(42, self.get.unredirected_hdrs["Content-length"])
        self.get.data = "spam"
        self.assertNotIn("Content-length", self.get.unredirected_hdrs)

    # issue 17485 same for deleting data.
    def test_deleting_data_should_remove_content_length(self):
        self.assertNotIn("Content-length", self.get.unredirected_hdrs)
        self.get.data = 'foo'
        self.get.add_unredirected_header("Content-length", 3)
        self.assertEqual(3, self.get.unredirected_hdrs["Content-length"])
        del self.get.data
        self.assertNotIn("Content-length", self.get.unredirected_hdrs)

    def test_get_full_url(self):
        self.assertEqual("http://www.python.org/~jeremy/",
                         self.get.get_full_url())

    def test_selector(self):
        self.assertEqual("/~jeremy/", self.get.selector)
        req = Request("http://www.python.org/")
        self.assertEqual("/", req.selector)

    def test_get_type(self):
        self.assertEqual("http", self.get.type)

    def test_get_host(self):
        self.assertEqual("www.python.org", self.get.host)

    def test_get_host_unquote(self):
        req = Request("http://www.%70ython.org/")
        self.assertEqual("www.python.org", req.host)

    def test_proxy(self):
        self.assertFalse(self.get.has_proxy())
        self.get.set_proxy("www.perl.org", "http")
        self.assertTrue(self.get.has_proxy())
        self.assertEqual("www.python.org", self.get.origin_req_host)
        self.assertEqual("www.perl.org", self.get.host)

    def test_wrapped_url(self):
        req = Request("<URL:http://www.python.org>")
        self.assertEqual("www.python.org", req.host)

    def test_url_fragment(self):
        req = Request("http://www.python.org/?qs=query#fragment=true")
        self.assertEqual("/?qs=query", req.selector)
        req = Request("http://www.python.org/#fun=true")
        self.assertEqual("/", req.selector)

        # Issue 11703: geturl() omits fragment in the original URL.
        url = 'http://docs.python.org/library/urllib2.html#OK'
        req = Request(url)
        self.assertEqual(req.get_full_url(), url)

    def test_url_fullurl_get_full_url(self):
        urls = ['http://docs.python.org',
                'http://docs.python.org/library/urllib2.html#OK',
                'http://www.python.org/?qs=query#fragment=true']
        for url in urls:
            req = Request(url)
            self.assertEqual(req.get_full_url(), req.full_url)


if __name__ == "__main__":
    unittest.main()