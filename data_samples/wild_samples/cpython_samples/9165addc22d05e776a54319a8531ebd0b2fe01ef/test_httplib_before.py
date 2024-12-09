                    if not line.rstrip(b"\r\n"):
                        break
                conn.sendall(b"HTTP/1.1 200 Connection established\r\n\r\n")
                nonlocal result
                result = reader.read()

        thread = threading.Thread(target=run_server)
        thread.start()
        self.addCleanup(thread.join, float(1))
        conn = client.HTTPConnection(*serv.getsockname())
        conn.request("CONNECT", "dummy:1234")
        response = conn.getresponse()
        try:
            self.assertEqual(response.status, client.OK)
            s = socket.socket(fileno=response.fileno())
            try:
                s.sendall(b"proxied data\n")
            finally:
                s.detach()
        finally:
            response.close()
            conn.close()
        thread.join()
        self.assertEqual(result, b"proxied data\n")

    def test_putrequest_override_validation(self):
        """
        It should be possible to override the default validation
        behavior in putrequest (bpo-38216).
        """
        class UnsafeHTTPConnection(client.HTTPConnection):
            def _validate_path(self, url):
                pass

        conn = UnsafeHTTPConnection('example.com')
        conn.sock = FakeSocket('')
        conn.putrequest('GET', '/\x00')

    def test_putrequest_override_encoding(self):
        """
        It should be possible to override the default encoding
        to transmit bytes in another encoding even if invalid
        (bpo-36274).
        """
        class UnsafeHTTPConnection(client.HTTPConnection):
            def _encode_request(self, str_url):
                return str_url.encode('utf-8')

        conn = UnsafeHTTPConnection('example.com')
        conn.sock = FakeSocket('')
        conn.putrequest('GET', '/☃')


class ExtendedReadTest(TestCase):
    """
    Test peek(), read1(), readline()
    """
    lines = (
        'HTTP/1.1 200 OK\r\n'
        '\r\n'
        'hello world!\n'
        'and now \n'
        'for something completely different\n'
        'foo'
        )
    lines_expected = lines[lines.find('hello'):].encode("ascii")
    lines_chunked = (
        'HTTP/1.1 200 OK\r\n'
        'Transfer-Encoding: chunked\r\n\r\n'
        'a\r\n'
        'hello worl\r\n'
        '3\r\n'
        'd!\n\r\n'
        '9\r\n'
        'and now \n\r\n'
        '23\r\n'
        'for something completely different\n\r\n'
        '3\r\n'
        'foo\r\n'
        '0\r\n' # terminating chunk
        '\r\n'  # end of trailers
    )

    def setUp(self):
        sock = FakeSocket(self.lines)
        resp = client.HTTPResponse(sock, method="GET")
        resp.begin()
        resp.fp = io.BufferedReader(resp.fp)
        self.resp = resp



    def test_peek(self):
        resp = self.resp
        # patch up the buffered peek so that it returns not too much stuff
        oldpeek = resp.fp.peek
        def mypeek(n=-1):
            p = oldpeek(n)
            if n >= 0:
                return p[:n]
            return p[:10]
        resp.fp.peek = mypeek

        all = []
        while True:
            # try a short peek
            p = resp.peek(3)
            if p:
                self.assertGreater(len(p), 0)
                # then unbounded peek
                p2 = resp.peek()
                self.assertGreaterEqual(len(p2), len(p))
                self.assertTrue(p2.startswith(p))
                next = resp.read(len(p2))
                self.assertEqual(next, p2)
            else:
                next = resp.read()
                self.assertFalse(next)
            all.append(next)
            if not next:
                break
        self.assertEqual(b"".join(all), self.lines_expected)

    def test_readline(self):
        resp = self.resp
        self._verify_readline(self.resp.readline, self.lines_expected)

    def _verify_readline(self, readline, expected):
        all = []
        while True:
            # short readlines
            line = readline(5)
            if line and line != b"foo":
                if len(line) < 5:
                    self.assertTrue(line.endswith(b"\n"))
            all.append(line)
            if not line:
                break
        self.assertEqual(b"".join(all), expected)

    def test_read1(self):
        resp = self.resp
        def r():
            res = resp.read1(4)
            self.assertLessEqual(len(res), 4)
            return res
        readliner = Readliner(r)
        self._verify_readline(readliner.readline, self.lines_expected)

    def test_read1_unbounded(self):
        resp = self.resp
        all = []
        while True:
            data = resp.read1()
            if not data:
                break
            all.append(data)
        self.assertEqual(b"".join(all), self.lines_expected)

    def test_read1_bounded(self):
        resp = self.resp
        all = []
        while True:
            data = resp.read1(10)
            if not data:
                break
            self.assertLessEqual(len(data), 10)
            all.append(data)
        self.assertEqual(b"".join(all), self.lines_expected)

    def test_read1_0(self):
        self.assertEqual(self.resp.read1(0), b"")

    def test_peek_0(self):
        p = self.resp.peek(0)
        self.assertLessEqual(0, len(p))


class ExtendedReadTestChunked(ExtendedReadTest):
    """
    Test peek(), read1(), readline() in chunked mode
    """
    lines = (
        'HTTP/1.1 200 OK\r\n'
        'Transfer-Encoding: chunked\r\n\r\n'
        'a\r\n'
        'hello worl\r\n'
        '3\r\n'
        'd!\n\r\n'
        '9\r\n'
        'and now \n\r\n'
        '23\r\n'
        'for something completely different\n\r\n'
        '3\r\n'
        'foo\r\n'
        '0\r\n' # terminating chunk
        '\r\n'  # end of trailers
    )


class Readliner:
    """
    a simple readline class that uses an arbitrary read function and buffering
    """
    def __init__(self, readfunc):
        self.readfunc = readfunc
        self.remainder = b""

    def readline(self, limit):
        data = []
        datalen = 0
        read = self.remainder
        try:
            while True:
                idx = read.find(b'\n')
                if idx != -1:
                    break
                if datalen + len(read) >= limit:
                    idx = limit - datalen - 1
                # read more data
                data.append(read)
                read = self.readfunc()
                if not read:
                    idx = 0 #eof condition
                    break
            idx += 1
            data.append(read[:idx])
            self.remainder = read[idx:]
            return b"".join(data)
        except:
            self.remainder = b"".join(data)
            raise


class OfflineTest(TestCase):
    def test_all(self):
        # Documented objects defined in the module should be in __all__
        expected = {"responses"}  # White-list documented dict() object
        # HTTPMessage, parse_headers(), and the HTTP status code constants are
        # intentionally omitted for simplicity
        blacklist = {"HTTPMessage", "parse_headers"}
        for name in dir(client):
            if name.startswith("_") or name in blacklist:
                continue
            module_object = getattr(client, name)
            if getattr(module_object, "__module__", None) == "http.client":
                expected.add(name)
        self.assertCountEqual(client.__all__, expected)

    def test_responses(self):
        self.assertEqual(client.responses[client.NOT_FOUND], "Not Found")

    def test_client_constants(self):
        # Make sure we don't break backward compatibility with 3.4
        expected = [
            'CONTINUE',
            'SWITCHING_PROTOCOLS',
            'PROCESSING',
            'OK',
            'CREATED',
            'ACCEPTED',
            'NON_AUTHORITATIVE_INFORMATION',
            'NO_CONTENT',
            'RESET_CONTENT',
            'PARTIAL_CONTENT',
            'MULTI_STATUS',
            'IM_USED',
            'MULTIPLE_CHOICES',
            'MOVED_PERMANENTLY',
            'FOUND',
            'SEE_OTHER',
            'NOT_MODIFIED',
            'USE_PROXY',
            'TEMPORARY_REDIRECT',
            'BAD_REQUEST',
            'UNAUTHORIZED',
            'PAYMENT_REQUIRED',
            'FORBIDDEN',
            'NOT_FOUND',
            'METHOD_NOT_ALLOWED',
            'NOT_ACCEPTABLE',
            'PROXY_AUTHENTICATION_REQUIRED',
            'REQUEST_TIMEOUT',
            'CONFLICT',
            'GONE',
            'LENGTH_REQUIRED',
            'PRECONDITION_FAILED',
            'REQUEST_ENTITY_TOO_LARGE',
            'REQUEST_URI_TOO_LONG',
            'UNSUPPORTED_MEDIA_TYPE',
            'REQUESTED_RANGE_NOT_SATISFIABLE',
            'EXPECTATION_FAILED',
            'MISDIRECTED_REQUEST',
            'UNPROCESSABLE_ENTITY',
            'LOCKED',
            'FAILED_DEPENDENCY',
            'UPGRADE_REQUIRED',
            'PRECONDITION_REQUIRED',
            'TOO_MANY_REQUESTS',
            'REQUEST_HEADER_FIELDS_TOO_LARGE',
            'UNAVAILABLE_FOR_LEGAL_REASONS',
            'INTERNAL_SERVER_ERROR',
            'NOT_IMPLEMENTED',
            'BAD_GATEWAY',
            'SERVICE_UNAVAILABLE',
            'GATEWAY_TIMEOUT',
            'HTTP_VERSION_NOT_SUPPORTED',
            'INSUFFICIENT_STORAGE',
            'NOT_EXTENDED',
            'NETWORK_AUTHENTICATION_REQUIRED',
            'EARLY_HINTS',
            'TOO_EARLY'
        ]
        for const in expected:
            with self.subTest(constant=const):
                self.assertTrue(hasattr(client, const))


class SourceAddressTest(TestCase):
    def setUp(self):
        self.serv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.port = support.bind_port(self.serv)
        self.source_port = support.find_unused_port()
        self.serv.listen()
        self.conn = None

    def tearDown(self):
        if self.conn:
            self.conn.close()
            self.conn = None
        self.serv.close()
        self.serv = None

    def testHTTPConnectionSourceAddress(self):
        self.conn = client.HTTPConnection(HOST, self.port,
                source_address=('', self.source_port))
        self.conn.connect()
        self.assertEqual(self.conn.sock.getsockname()[1], self.source_port)

    @unittest.skipIf(not hasattr(client, 'HTTPSConnection'),
                     'http.client.HTTPSConnection not defined')
    def testHTTPSConnectionSourceAddress(self):
        self.conn = client.HTTPSConnection(HOST, self.port,
                source_address=('', self.source_port))
        # We don't test anything here other than the constructor not barfing as
        # this code doesn't deal with setting up an active running SSL server
        # for an ssl_wrapped connect() to actually return from.


class TimeoutTest(TestCase):
    PORT = None

    def setUp(self):
        self.serv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        TimeoutTest.PORT = support.bind_port(self.serv)
        self.serv.listen()

    def tearDown(self):
        self.serv.close()
        self.serv = None

    def testTimeoutAttribute(self):
        # This will prove that the timeout gets through HTTPConnection
        # and into the socket.

        # default -- use global socket timeout
        self.assertIsNone(socket.getdefaulttimeout())
        socket.setdefaulttimeout(30)
        try:
            httpConn = client.HTTPConnection(HOST, TimeoutTest.PORT)
            httpConn.connect()
        finally:
            socket.setdefaulttimeout(None)
        self.assertEqual(httpConn.sock.gettimeout(), 30)
        httpConn.close()

        # no timeout -- do not use global socket default
        self.assertIsNone(socket.getdefaulttimeout())
        socket.setdefaulttimeout(30)
        try:
            httpConn = client.HTTPConnection(HOST, TimeoutTest.PORT,
                                              timeout=None)
            httpConn.connect()
        finally:
            socket.setdefaulttimeout(None)
        self.assertEqual(httpConn.sock.gettimeout(), None)
        httpConn.close()

        # a value
        httpConn = client.HTTPConnection(HOST, TimeoutTest.PORT, timeout=30)
        httpConn.connect()
        self.assertEqual(httpConn.sock.gettimeout(), 30)
        httpConn.close()


class PersistenceTest(TestCase):

    def test_reuse_reconnect(self):
        # Should reuse or reconnect depending on header from server
        tests = (
            ('1.0', '', False),
            ('1.0', 'Connection: keep-alive\r\n', True),
            ('1.1', '', True),
            ('1.1', 'Connection: close\r\n', False),
            ('1.0', 'Connection: keep-ALIVE\r\n', True),
            ('1.1', 'Connection: cloSE\r\n', False),
        )
        for version, header, reuse in tests:
            with self.subTest(version=version, header=header):
                msg = (
                    'HTTP/{} 200 OK\r\n'
                    '{}'
                    'Content-Length: 12\r\n'
                    '\r\n'
                    'Dummy body\r\n'
                ).format(version, header)
                conn = FakeSocketHTTPConnection(msg)
                self.assertIsNone(conn.sock)
                conn.request('GET', '/open-connection')
                with conn.getresponse() as response:
                    self.assertEqual(conn.sock is None, not reuse)
                    response.read()
                self.assertEqual(conn.sock is None, not reuse)
                self.assertEqual(conn.connections, 1)
                conn.request('GET', '/subsequent-request')
                self.assertEqual(conn.connections, 1 if reuse else 2)

    def test_disconnected(self):

        def make_reset_reader(text):
            """Return BufferedReader that raises ECONNRESET at EOF"""
            stream = io.BytesIO(text)
            def readinto(buffer):
                size = io.BytesIO.readinto(stream, buffer)
                if size == 0:
                    raise ConnectionResetError()
                return size
            stream.readinto = readinto
            return io.BufferedReader(stream)

        tests = (
            (io.BytesIO, client.RemoteDisconnected),
            (make_reset_reader, ConnectionResetError),
        )
        for stream_factory, exception in tests:
            with self.subTest(exception=exception):
                conn = FakeSocketHTTPConnection(b'', stream_factory)
                conn.request('GET', '/eof-response')
                self.assertRaises(exception, conn.getresponse)
                self.assertIsNone(conn.sock)
                # HTTPConnection.connect() should be automatically invoked
                conn.request('GET', '/reconnect')
                self.assertEqual(conn.connections, 2)

    def test_100_close(self):
        conn = FakeSocketHTTPConnection(
            b'HTTP/1.1 100 Continue\r\n'
            b'\r\n'
            # Missing final response
        )
        conn.request('GET', '/', headers={'Expect': '100-continue'})
        self.assertRaises(client.RemoteDisconnected, conn.getresponse)
        self.assertIsNone(conn.sock)
        conn.request('GET', '/reconnect')
        self.assertEqual(conn.connections, 2)


class HTTPSTest(TestCase):

    def setUp(self):
        if not hasattr(client, 'HTTPSConnection'):
            self.skipTest('ssl support required')

    def make_server(self, certfile):
        from test.ssl_servers import make_https_server
        return make_https_server(self, certfile=certfile)

    def test_attributes(self):
        # simple test to check it's storing the timeout
        h = client.HTTPSConnection(HOST, TimeoutTest.PORT, timeout=30)
        self.assertEqual(h.timeout, 30)

    def test_networked(self):
        # Default settings: requires a valid cert from a trusted CA
        import ssl
        support.requires('network')
        with support.transient_internet('self-signed.pythontest.net'):
            h = client.HTTPSConnection('self-signed.pythontest.net', 443)
            with self.assertRaises(ssl.SSLError) as exc_info:
                h.request('GET', '/')
            self.assertEqual(exc_info.exception.reason, 'CERTIFICATE_VERIFY_FAILED')

    def test_networked_noverification(self):
        # Switch off cert verification
        import ssl
        support.requires('network')
        with support.transient_internet('self-signed.pythontest.net'):
            context = ssl._create_unverified_context()
            h = client.HTTPSConnection('self-signed.pythontest.net', 443,
                                       context=context)
            h.request('GET', '/')
            resp = h.getresponse()
            h.close()
            self.assertIn('nginx', resp.getheader('server'))
            resp.close()

    @support.system_must_validate_cert
    def test_networked_trusted_by_default_cert(self):
        # Default settings: requires a valid cert from a trusted CA
        support.requires('network')
        with support.transient_internet('www.python.org'):
            h = client.HTTPSConnection('www.python.org', 443)
            h.request('GET', '/')
            resp = h.getresponse()
            content_type = resp.getheader('content-type')
            resp.close()
            h.close()
            self.assertIn('text/html', content_type)

    def test_networked_good_cert(self):
        # We feed the server's cert as a validating cert
        import ssl
        support.requires('network')
        selfsigned_pythontestdotnet = 'self-signed.pythontest.net'
        with support.transient_internet(selfsigned_pythontestdotnet):
            context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
            self.assertEqual(context.verify_mode, ssl.CERT_REQUIRED)
            self.assertEqual(context.check_hostname, True)
            context.load_verify_locations(CERT_selfsigned_pythontestdotnet)
            try:
                h = client.HTTPSConnection(selfsigned_pythontestdotnet, 443,
                                           context=context)
                h.request('GET', '/')
                resp = h.getresponse()
            except ssl.SSLError as ssl_err:
                ssl_err_str = str(ssl_err)
                # In the error message of [SSL: CERTIFICATE_VERIFY_FAILED] on
                # modern Linux distros (Debian Buster, etc) default OpenSSL
                # configurations it'll fail saying "key too weak" until we
                # address https://bugs.python.org/issue36816 to use a proper
                # key size on self-signed.pythontest.net.
                if re.search(r'(?i)key.too.weak', ssl_err_str):
                    raise unittest.SkipTest(
                        f'Got {ssl_err_str} trying to connect '
                        f'to {selfsigned_pythontestdotnet}. '
                        'See https://bugs.python.org/issue36816.')
                raise
            server_string = resp.getheader('server')
            resp.close()
            h.close()
            self.assertIn('nginx', server_string)

    def test_networked_bad_cert(self):
        # We feed a "CA" cert that is unrelated to the server's cert
        import ssl
        support.requires('network')
        with support.transient_internet('self-signed.pythontest.net'):
            context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
            context.load_verify_locations(CERT_localhost)
            h = client.HTTPSConnection('self-signed.pythontest.net', 443, context=context)
            with self.assertRaises(ssl.SSLError) as exc_info:
                h.request('GET', '/')
            self.assertEqual(exc_info.exception.reason, 'CERTIFICATE_VERIFY_FAILED')

    def test_local_unknown_cert(self):
        # The custom cert isn't known to the default trust bundle
        import ssl
        server = self.make_server(CERT_localhost)
        h = client.HTTPSConnection('localhost', server.port)
        with self.assertRaises(ssl.SSLError) as exc_info:
            h.request('GET', '/')
        self.assertEqual(exc_info.exception.reason, 'CERTIFICATE_VERIFY_FAILED')

    def test_local_good_hostname(self):
        # The (valid) cert validates the HTTP hostname
        import ssl
        server = self.make_server(CERT_localhost)
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        context.load_verify_locations(CERT_localhost)
        h = client.HTTPSConnection('localhost', server.port, context=context)
        self.addCleanup(h.close)
        h.request('GET', '/nonexistent')
        resp = h.getresponse()
        self.addCleanup(resp.close)
        self.assertEqual(resp.status, 404)

    def test_local_bad_hostname(self):
        # The (valid) cert doesn't validate the HTTP hostname
        import ssl
        server = self.make_server(CERT_fakehostname)
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        context.load_verify_locations(CERT_fakehostname)
        h = client.HTTPSConnection('localhost', server.port, context=context)
        with self.assertRaises(ssl.CertificateError):
            h.request('GET', '/')
        # Same with explicit check_hostname=True
        with support.check_warnings(('', DeprecationWarning)):
            h = client.HTTPSConnection('localhost', server.port,
                                       context=context, check_hostname=True)
        with self.assertRaises(ssl.CertificateError):
            h.request('GET', '/')
        # With check_hostname=False, the mismatching is ignored
        context.check_hostname = False
        with support.check_warnings(('', DeprecationWarning)):
            h = client.HTTPSConnection('localhost', server.port,
                                       context=context, check_hostname=False)
        h.request('GET', '/nonexistent')
        resp = h.getresponse()
        resp.close()
        h.close()
        self.assertEqual(resp.status, 404)
        # The context's check_hostname setting is used if one isn't passed to
        # HTTPSConnection.
        context.check_hostname = False
        h = client.HTTPSConnection('localhost', server.port, context=context)
        h.request('GET', '/nonexistent')
        resp = h.getresponse()
        self.assertEqual(resp.status, 404)
        resp.close()
        h.close()
        # Passing check_hostname to HTTPSConnection should override the
        # context's setting.
        with support.check_warnings(('', DeprecationWarning)):
            h = client.HTTPSConnection('localhost', server.port,
                                       context=context, check_hostname=True)
        with self.assertRaises(ssl.CertificateError):
            h.request('GET', '/')

    @unittest.skipIf(not hasattr(client, 'HTTPSConnection'),
                     'http.client.HTTPSConnection not available')
    def test_host_port(self):
        # Check invalid host_port

        for hp in ("www.python.org:abc", "user:password@www.python.org"):
            self.assertRaises(client.InvalidURL, client.HTTPSConnection, hp)

        for hp, h, p in (("[fe80::207:e9ff:fe9b]:8000",
                          "fe80::207:e9ff:fe9b", 8000),
                         ("www.python.org:443", "www.python.org", 443),
                         ("www.python.org:", "www.python.org", 443),
                         ("www.python.org", "www.python.org", 443),
                         ("[fe80::207:e9ff:fe9b]", "fe80::207:e9ff:fe9b", 443),
                         ("[fe80::207:e9ff:fe9b]:", "fe80::207:e9ff:fe9b",
                             443)):
            c = client.HTTPSConnection(hp)
            self.assertEqual(h, c.host)
            self.assertEqual(p, c.port)

    def test_tls13_pha(self):
        import ssl
        if not ssl.HAS_TLSv1_3:
            self.skipTest('TLS 1.3 support required')
        # just check status of PHA flag
        h = client.HTTPSConnection('localhost', 443)
        self.assertTrue(h._context.post_handshake_auth)

        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        self.assertFalse(context.post_handshake_auth)
        h = client.HTTPSConnection('localhost', 443, context=context)
        self.assertIs(h._context, context)
        self.assertFalse(h._context.post_handshake_auth)

        with warnings.catch_warnings():
            warnings.filterwarnings('ignore', 'key_file, cert_file and check_hostname are deprecated',
                                    DeprecationWarning)
            h = client.HTTPSConnection('localhost', 443, context=context,
                                       cert_file=CERT_localhost)
        self.assertTrue(h._context.post_handshake_auth)


class RequestBodyTest(TestCase):
    """Test cases where a request includes a message body."""

    def setUp(self):
        self.conn = client.HTTPConnection('example.com')
        self.conn.sock = self.sock = FakeSocket("")
        self.conn.sock = self.sock

    def get_headers_and_fp(self):
        f = io.BytesIO(self.sock.data)
        f.readline()  # read the request line
        message = client.parse_headers(f)
        return message, f

    def test_list_body(self):
        # Note that no content-length is automatically calculated for
        # an iterable.  The request will fall back to send chunked
        # transfer encoding.
        cases = (
            ([b'foo', b'bar'], b'3\r\nfoo\r\n3\r\nbar\r\n0\r\n\r\n'),
            ((b'foo', b'bar'), b'3\r\nfoo\r\n3\r\nbar\r\n0\r\n\r\n'),
        )
        for body, expected in cases:
            with self.subTest(body):
                self.conn = client.HTTPConnection('example.com')
                self.conn.sock = self.sock = FakeSocket('')

                self.conn.request('PUT', '/url', body)
                msg, f = self.get_headers_and_fp()
                self.assertNotIn('Content-Type', msg)
                self.assertNotIn('Content-Length', msg)
                self.assertEqual(msg.get('Transfer-Encoding'), 'chunked')
                self.assertEqual(expected, f.read())

    def test_manual_content_length(self):
        # Set an incorrect content-length so that we can verify that
        # it will not be over-ridden by the library.
        self.conn.request("PUT", "/url", "body",
                          {"Content-Length": "42"})
        message, f = self.get_headers_and_fp()
        self.assertEqual("42", message.get("content-length"))
        self.assertEqual(4, len(f.read()))

    def test_ascii_body(self):
        self.conn.request("PUT", "/url", "body")
        message, f = self.get_headers_and_fp()
        self.assertEqual("text/plain", message.get_content_type())
        self.assertIsNone(message.get_charset())
        self.assertEqual("4", message.get("content-length"))
        self.assertEqual(b'body', f.read())

    def test_latin1_body(self):
        self.conn.request("PUT", "/url", "body\xc1")
        message, f = self.get_headers_and_fp()
        self.assertEqual("text/plain", message.get_content_type())
        self.assertIsNone(message.get_charset())
        self.assertEqual("5", message.get("content-length"))
        self.assertEqual(b'body\xc1', f.read())

    def test_bytes_body(self):
        self.conn.request("PUT", "/url", b"body\xc1")
        message, f = self.get_headers_and_fp()
        self.assertEqual("text/plain", message.get_content_type())
        self.assertIsNone(message.get_charset())
        self.assertEqual("5", message.get("content-length"))
        self.assertEqual(b'body\xc1', f.read())

    def test_text_file_body(self):
        self.addCleanup(support.unlink, support.TESTFN)
        with open(support.TESTFN, "w") as f:
            f.write("body")
        with open(support.TESTFN) as f:
            self.conn.request("PUT", "/url", f)
            message, f = self.get_headers_and_fp()
            self.assertEqual("text/plain", message.get_content_type())
            self.assertIsNone(message.get_charset())
            # No content-length will be determined for files; the body
            # will be sent using chunked transfer encoding instead.
            self.assertIsNone(message.get("content-length"))
            self.assertEqual("chunked", message.get("transfer-encoding"))
            self.assertEqual(b'4\r\nbody\r\n0\r\n\r\n', f.read())

    def test_binary_file_body(self):
        self.addCleanup(support.unlink, support.TESTFN)
        with open(support.TESTFN, "wb") as f:
            f.write(b"body\xc1")
        with open(support.TESTFN, "rb") as f:
            self.conn.request("PUT", "/url", f)
            message, f = self.get_headers_and_fp()
            self.assertEqual("text/plain", message.get_content_type())
            self.assertIsNone(message.get_charset())
            self.assertEqual("chunked", message.get("Transfer-Encoding"))
            self.assertNotIn("Content-Length", message)
            self.assertEqual(b'5\r\nbody\xc1\r\n0\r\n\r\n', f.read())


class HTTPResponseTest(TestCase):

    def setUp(self):
        body = "HTTP/1.1 200 Ok\r\nMy-Header: first-value\r\nMy-Header: \
                second-value\r\n\r\nText"
        sock = FakeSocket(body)
        self.resp = client.HTTPResponse(sock)
        self.resp.begin()

    def test_getting_header(self):
        header = self.resp.getheader('My-Header')
        self.assertEqual(header, 'first-value, second-value')

        header = self.resp.getheader('My-Header', 'some default')
        self.assertEqual(header, 'first-value, second-value')

    def test_getting_nonexistent_header_with_string_default(self):
        header = self.resp.getheader('No-Such-Header', 'default-value')
        self.assertEqual(header, 'default-value')

    def test_getting_nonexistent_header_with_iterable_default(self):
        header = self.resp.getheader('No-Such-Header', ['default', 'values'])
        self.assertEqual(header, 'default, values')

        header = self.resp.getheader('No-Such-Header', ('default', 'values'))
        self.assertEqual(header, 'default, values')

    def test_getting_nonexistent_header_without_default(self):
        header = self.resp.getheader('No-Such-Header')
        self.assertEqual(header, None)

    def test_getting_header_defaultint(self):
        header = self.resp.getheader('No-Such-Header',default=42)
        self.assertEqual(header, 42)

class TunnelTests(TestCase):
    def setUp(self):
        response_text = (
            'HTTP/1.0 200 OK\r\n\r\n' # Reply to CONNECT
            'HTTP/1.1 200 OK\r\n' # Reply to HEAD
            'Content-Length: 42\r\n\r\n'
        )
        self.host = 'proxy.com'
        self.conn = client.HTTPConnection(self.host)
        self.conn._create_connection = self._create_connection(response_text)

    def tearDown(self):
        self.conn.close()

    def _create_connection(self, response_text):
        def create_connection(address, timeout=None, source_address=None):
            return FakeSocket(response_text, host=address[0], port=address[1])
        return create_connection

    def test_set_tunnel_host_port_headers(self):
        tunnel_host = 'destination.com'
        tunnel_port = 8888
        tunnel_headers = {'User-Agent': 'Mozilla/5.0 (compatible, MSIE 11)'}
        self.conn.set_tunnel(tunnel_host, port=tunnel_port,
                             headers=tunnel_headers)
        self.conn.request('HEAD', '/', '')
        self.assertEqual(self.conn.sock.host, self.host)
        self.assertEqual(self.conn.sock.port, client.HTTP_PORT)
        self.assertEqual(self.conn._tunnel_host, tunnel_host)
        self.assertEqual(self.conn._tunnel_port, tunnel_port)
        self.assertEqual(self.conn._tunnel_headers, tunnel_headers)

    def test_disallow_set_tunnel_after_connect(self):
        # Once connected, we shouldn't be able to tunnel anymore
        self.conn.connect()
        self.assertRaises(RuntimeError, self.conn.set_tunnel,
                          'destination.com')

    def test_connect_with_tunnel(self):
        self.conn.set_tunnel('destination.com')
        self.conn.request('HEAD', '/', '')
        self.assertEqual(self.conn.sock.host, self.host)
        self.assertEqual(self.conn.sock.port, client.HTTP_PORT)
        self.assertIn(b'CONNECT destination.com', self.conn.sock.data)
        # issue22095
        self.assertNotIn(b'Host: destination.com:None', self.conn.sock.data)
        self.assertIn(b'Host: destination.com', self.conn.sock.data)

        # This test should be removed when CONNECT gets the HTTP/1.1 blessing
        self.assertNotIn(b'Host: proxy.com', self.conn.sock.data)

    def test_connect_put_request(self):
        self.conn.set_tunnel('destination.com')
        self.conn.request('PUT', '/', '')
        self.assertEqual(self.conn.sock.host, self.host)
        self.assertEqual(self.conn.sock.port, client.HTTP_PORT)
        self.assertIn(b'CONNECT destination.com', self.conn.sock.data)
        self.assertIn(b'Host: destination.com', self.conn.sock.data)

    def test_tunnel_debuglog(self):
        expected_header = 'X-Dummy: 1'
        response_text = 'HTTP/1.0 200 OK\r\n{}\r\n\r\n'.format(expected_header)

        self.conn.set_debuglevel(1)
        self.conn._create_connection = self._create_connection(response_text)
        self.conn.set_tunnel('destination.com')

        with support.captured_stdout() as output:
            self.conn.request('PUT', '/', '')
        lines = output.getvalue().splitlines()
        self.assertIn('header: {}'.format(expected_header), lines)


if __name__ == '__main__':
    unittest.main(verbosity=2)
            def _validate_path(self, url):
                pass

        conn = UnsafeHTTPConnection('example.com')
        conn.sock = FakeSocket('')
        conn.putrequest('GET', '/\x00')

    def test_putrequest_override_encoding(self):
        """
        It should be possible to override the default encoding
        to transmit bytes in another encoding even if invalid
        (bpo-36274).
        """
        class UnsafeHTTPConnection(client.HTTPConnection):
            def _encode_request(self, str_url):
                return str_url.encode('utf-8')

        conn = UnsafeHTTPConnection('example.com')
        conn.sock = FakeSocket('')
        conn.putrequest('GET', '/☃')


class ExtendedReadTest(TestCase):
    """
    Test peek(), read1(), readline()
    """
    lines = (
        'HTTP/1.1 200 OK\r\n'
        '\r\n'
        'hello world!\n'
        'and now \n'
        'for something completely different\n'
        'foo'
        )
    lines_expected = lines[lines.find('hello'):].encode("ascii")
    lines_chunked = (
        'HTTP/1.1 200 OK\r\n'
        'Transfer-Encoding: chunked\r\n\r\n'
        'a\r\n'
        'hello worl\r\n'
        '3\r\n'
        'd!\n\r\n'
        '9\r\n'
        'and now \n\r\n'
        '23\r\n'
        'for something completely different\n\r\n'
        '3\r\n'
        'foo\r\n'
        '0\r\n' # terminating chunk
        '\r\n'  # end of trailers
    )

    def setUp(self):
        sock = FakeSocket(self.lines)
        resp = client.HTTPResponse(sock, method="GET")
        resp.begin()
        resp.fp = io.BufferedReader(resp.fp)
        self.resp = resp



    def test_peek(self):
        resp = self.resp
        # patch up the buffered peek so that it returns not too much stuff
        oldpeek = resp.fp.peek
        def mypeek(n=-1):
            p = oldpeek(n)
            if n >= 0:
                return p[:n]
            return p[:10]
        resp.fp.peek = mypeek

        all = []
        while True:
            # try a short peek
            p = resp.peek(3)
            if p:
                self.assertGreater(len(p), 0)
                # then unbounded peek
                p2 = resp.peek()
                self.assertGreaterEqual(len(p2), len(p))
                self.assertTrue(p2.startswith(p))
                next = resp.read(len(p2))
                self.assertEqual(next, p2)
            else:
                next = resp.read()
                self.assertFalse(next)
            all.append(next)
            if not next:
                break
        self.assertEqual(b"".join(all), self.lines_expected)

    def test_readline(self):
        resp = self.resp
        self._verify_readline(self.resp.readline, self.lines_expected)

    def _verify_readline(self, readline, expected):
        all = []
        while True:
            # short readlines
            line = readline(5)
            if line and line != b"foo":
                if len(line) < 5:
                    self.assertTrue(line.endswith(b"\n"))
            all.append(line)
            if not line:
                break
        self.assertEqual(b"".join(all), expected)

    def test_read1(self):
        resp = self.resp
        def r():
            res = resp.read1(4)
            self.assertLessEqual(len(res), 4)
            return res
        readliner = Readliner(r)
        self._verify_readline(readliner.readline, self.lines_expected)

    def test_read1_unbounded(self):
        resp = self.resp
        all = []
        while True:
            data = resp.read1()
            if not data:
                break
            all.append(data)
        self.assertEqual(b"".join(all), self.lines_expected)

    def test_read1_bounded(self):
        resp = self.resp
        all = []
        while True:
            data = resp.read1(10)
            if not data:
                break
            self.assertLessEqual(len(data), 10)
            all.append(data)
        self.assertEqual(b"".join(all), self.lines_expected)

    def test_read1_0(self):
        self.assertEqual(self.resp.read1(0), b"")

    def test_peek_0(self):
        p = self.resp.peek(0)
        self.assertLessEqual(0, len(p))


class ExtendedReadTestChunked(ExtendedReadTest):
    """
    Test peek(), read1(), readline() in chunked mode
    """
    lines = (
        'HTTP/1.1 200 OK\r\n'
        'Transfer-Encoding: chunked\r\n\r\n'
        'a\r\n'
        'hello worl\r\n'
        '3\r\n'
        'd!\n\r\n'
        '9\r\n'
        'and now \n\r\n'
        '23\r\n'
        'for something completely different\n\r\n'
        '3\r\n'
        'foo\r\n'
        '0\r\n' # terminating chunk
        '\r\n'  # end of trailers
    )


class Readliner:
    """
    a simple readline class that uses an arbitrary read function and buffering
    """
    def __init__(self, readfunc):
        self.readfunc = readfunc
        self.remainder = b""

    def readline(self, limit):
        data = []
        datalen = 0
        read = self.remainder
        try:
            while True:
                idx = read.find(b'\n')
                if idx != -1:
                    break
                if datalen + len(read) >= limit:
                    idx = limit - datalen - 1
                # read more data
                data.append(read)
                read = self.readfunc()
                if not read:
                    idx = 0 #eof condition
                    break
            idx += 1
            data.append(read[:idx])
            self.remainder = read[idx:]
            return b"".join(data)
        except:
            self.remainder = b"".join(data)
            raise


class OfflineTest(TestCase):
    def test_all(self):
        # Documented objects defined in the module should be in __all__
        expected = {"responses"}  # White-list documented dict() object
        # HTTPMessage, parse_headers(), and the HTTP status code constants are
        # intentionally omitted for simplicity
        blacklist = {"HTTPMessage", "parse_headers"}
        for name in dir(client):
            if name.startswith("_") or name in blacklist:
                continue
            module_object = getattr(client, name)
            if getattr(module_object, "__module__", None) == "http.client":
                expected.add(name)
        self.assertCountEqual(client.__all__, expected)

    def test_responses(self):
        self.assertEqual(client.responses[client.NOT_FOUND], "Not Found")

    def test_client_constants(self):
        # Make sure we don't break backward compatibility with 3.4
        expected = [
            'CONTINUE',
            'SWITCHING_PROTOCOLS',
            'PROCESSING',
            'OK',
            'CREATED',
            'ACCEPTED',
            'NON_AUTHORITATIVE_INFORMATION',
            'NO_CONTENT',
            'RESET_CONTENT',
            'PARTIAL_CONTENT',
            'MULTI_STATUS',
            'IM_USED',
            'MULTIPLE_CHOICES',
            'MOVED_PERMANENTLY',
            'FOUND',
            'SEE_OTHER',
            'NOT_MODIFIED',
            'USE_PROXY',
            'TEMPORARY_REDIRECT',
            'BAD_REQUEST',
            'UNAUTHORIZED',
            'PAYMENT_REQUIRED',
            'FORBIDDEN',
            'NOT_FOUND',
            'METHOD_NOT_ALLOWED',
            'NOT_ACCEPTABLE',
            'PROXY_AUTHENTICATION_REQUIRED',
            'REQUEST_TIMEOUT',
            'CONFLICT',
            'GONE',
            'LENGTH_REQUIRED',
            'PRECONDITION_FAILED',
            'REQUEST_ENTITY_TOO_LARGE',
            'REQUEST_URI_TOO_LONG',
            'UNSUPPORTED_MEDIA_TYPE',
            'REQUESTED_RANGE_NOT_SATISFIABLE',
            'EXPECTATION_FAILED',
            'MISDIRECTED_REQUEST',
            'UNPROCESSABLE_ENTITY',
            'LOCKED',
            'FAILED_DEPENDENCY',
            'UPGRADE_REQUIRED',
            'PRECONDITION_REQUIRED',
            'TOO_MANY_REQUESTS',
            'REQUEST_HEADER_FIELDS_TOO_LARGE',
            'UNAVAILABLE_FOR_LEGAL_REASONS',
            'INTERNAL_SERVER_ERROR',
            'NOT_IMPLEMENTED',
            'BAD_GATEWAY',
            'SERVICE_UNAVAILABLE',
            'GATEWAY_TIMEOUT',
            'HTTP_VERSION_NOT_SUPPORTED',
            'INSUFFICIENT_STORAGE',
            'NOT_EXTENDED',
            'NETWORK_AUTHENTICATION_REQUIRED',
            'EARLY_HINTS',
            'TOO_EARLY'
        ]
        for const in expected:
            with self.subTest(constant=const):
                self.assertTrue(hasattr(client, const))


class SourceAddressTest(TestCase):
    def setUp(self):
        self.serv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.port = support.bind_port(self.serv)
        self.source_port = support.find_unused_port()
        self.serv.listen()
        self.conn = None

    def tearDown(self):
        if self.conn:
            self.conn.close()
            self.conn = None
        self.serv.close()
        self.serv = None

    def testHTTPConnectionSourceAddress(self):
        self.conn = client.HTTPConnection(HOST, self.port,
                source_address=('', self.source_port))
        self.conn.connect()
        self.assertEqual(self.conn.sock.getsockname()[1], self.source_port)

    @unittest.skipIf(not hasattr(client, 'HTTPSConnection'),
                     'http.client.HTTPSConnection not defined')
    def testHTTPSConnectionSourceAddress(self):
        self.conn = client.HTTPSConnection(HOST, self.port,
                source_address=('', self.source_port))
        # We don't test anything here other than the constructor not barfing as
        # this code doesn't deal with setting up an active running SSL server
        # for an ssl_wrapped connect() to actually return from.


class TimeoutTest(TestCase):
    PORT = None

    def setUp(self):
        self.serv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        TimeoutTest.PORT = support.bind_port(self.serv)
        self.serv.listen()

    def tearDown(self):
        self.serv.close()
        self.serv = None

    def testTimeoutAttribute(self):
        # This will prove that the timeout gets through HTTPConnection
        # and into the socket.

        # default -- use global socket timeout
        self.assertIsNone(socket.getdefaulttimeout())
        socket.setdefaulttimeout(30)
        try:
            httpConn = client.HTTPConnection(HOST, TimeoutTest.PORT)
            httpConn.connect()
        finally:
            socket.setdefaulttimeout(None)
        self.assertEqual(httpConn.sock.gettimeout(), 30)
        httpConn.close()

        # no timeout -- do not use global socket default
        self.assertIsNone(socket.getdefaulttimeout())
        socket.setdefaulttimeout(30)
        try:
            httpConn = client.HTTPConnection(HOST, TimeoutTest.PORT,
                                              timeout=None)
            httpConn.connect()
        finally:
            socket.setdefaulttimeout(None)
        self.assertEqual(httpConn.sock.gettimeout(), None)
        httpConn.close()

        # a value
        httpConn = client.HTTPConnection(HOST, TimeoutTest.PORT, timeout=30)
        httpConn.connect()
        self.assertEqual(httpConn.sock.gettimeout(), 30)
        httpConn.close()


class PersistenceTest(TestCase):

    def test_reuse_reconnect(self):
        # Should reuse or reconnect depending on header from server
        tests = (
            ('1.0', '', False),
            ('1.0', 'Connection: keep-alive\r\n', True),
            ('1.1', '', True),
            ('1.1', 'Connection: close\r\n', False),
            ('1.0', 'Connection: keep-ALIVE\r\n', True),
            ('1.1', 'Connection: cloSE\r\n', False),
        )
        for version, header, reuse in tests:
            with self.subTest(version=version, header=header):
                msg = (
                    'HTTP/{} 200 OK\r\n'
                    '{}'
                    'Content-Length: 12\r\n'
                    '\r\n'
                    'Dummy body\r\n'
                ).format(version, header)
                conn = FakeSocketHTTPConnection(msg)
                self.assertIsNone(conn.sock)
                conn.request('GET', '/open-connection')
                with conn.getresponse() as response:
                    self.assertEqual(conn.sock is None, not reuse)
                    response.read()
                self.assertEqual(conn.sock is None, not reuse)
                self.assertEqual(conn.connections, 1)
                conn.request('GET', '/subsequent-request')
                self.assertEqual(conn.connections, 1 if reuse else 2)

    def test_disconnected(self):

        def make_reset_reader(text):
            """Return BufferedReader that raises ECONNRESET at EOF"""
            stream = io.BytesIO(text)
            def readinto(buffer):
                size = io.BytesIO.readinto(stream, buffer)
                if size == 0:
                    raise ConnectionResetError()
                return size
            stream.readinto = readinto
            return io.BufferedReader(stream)

        tests = (
            (io.BytesIO, client.RemoteDisconnected),
            (make_reset_reader, ConnectionResetError),
        )
        for stream_factory, exception in tests:
            with self.subTest(exception=exception):
                conn = FakeSocketHTTPConnection(b'', stream_factory)
                conn.request('GET', '/eof-response')
                self.assertRaises(exception, conn.getresponse)
                self.assertIsNone(conn.sock)
                # HTTPConnection.connect() should be automatically invoked
                conn.request('GET', '/reconnect')
                self.assertEqual(conn.connections, 2)

    def test_100_close(self):
        conn = FakeSocketHTTPConnection(
            b'HTTP/1.1 100 Continue\r\n'
            b'\r\n'
            # Missing final response
        )
        conn.request('GET', '/', headers={'Expect': '100-continue'})
        self.assertRaises(client.RemoteDisconnected, conn.getresponse)
        self.assertIsNone(conn.sock)
        conn.request('GET', '/reconnect')
        self.assertEqual(conn.connections, 2)


class HTTPSTest(TestCase):

    def setUp(self):
        if not hasattr(client, 'HTTPSConnection'):
            self.skipTest('ssl support required')

    def make_server(self, certfile):
        from test.ssl_servers import make_https_server
        return make_https_server(self, certfile=certfile)

    def test_attributes(self):
        # simple test to check it's storing the timeout
        h = client.HTTPSConnection(HOST, TimeoutTest.PORT, timeout=30)
        self.assertEqual(h.timeout, 30)

    def test_networked(self):
        # Default settings: requires a valid cert from a trusted CA
        import ssl
        support.requires('network')
        with support.transient_internet('self-signed.pythontest.net'):
            h = client.HTTPSConnection('self-signed.pythontest.net', 443)
            with self.assertRaises(ssl.SSLError) as exc_info:
                h.request('GET', '/')
            self.assertEqual(exc_info.exception.reason, 'CERTIFICATE_VERIFY_FAILED')

    def test_networked_noverification(self):
        # Switch off cert verification
        import ssl
        support.requires('network')
        with support.transient_internet('self-signed.pythontest.net'):
            context = ssl._create_unverified_context()
            h = client.HTTPSConnection('self-signed.pythontest.net', 443,
                                       context=context)
            h.request('GET', '/')
            resp = h.getresponse()
            h.close()
            self.assertIn('nginx', resp.getheader('server'))
            resp.close()

    @support.system_must_validate_cert
    def test_networked_trusted_by_default_cert(self):
        # Default settings: requires a valid cert from a trusted CA
        support.requires('network')
        with support.transient_internet('www.python.org'):
            h = client.HTTPSConnection('www.python.org', 443)
            h.request('GET', '/')
            resp = h.getresponse()
            content_type = resp.getheader('content-type')
            resp.close()
            h.close()
            self.assertIn('text/html', content_type)

    def test_networked_good_cert(self):
        # We feed the server's cert as a validating cert
        import ssl
        support.requires('network')
        selfsigned_pythontestdotnet = 'self-signed.pythontest.net'
        with support.transient_internet(selfsigned_pythontestdotnet):
            context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
            self.assertEqual(context.verify_mode, ssl.CERT_REQUIRED)
            self.assertEqual(context.check_hostname, True)
            context.load_verify_locations(CERT_selfsigned_pythontestdotnet)
            try:
                h = client.HTTPSConnection(selfsigned_pythontestdotnet, 443,
                                           context=context)
                h.request('GET', '/')
                resp = h.getresponse()
            except ssl.SSLError as ssl_err:
                ssl_err_str = str(ssl_err)
                # In the error message of [SSL: CERTIFICATE_VERIFY_FAILED] on
                # modern Linux distros (Debian Buster, etc) default OpenSSL
                # configurations it'll fail saying "key too weak" until we
                # address https://bugs.python.org/issue36816 to use a proper
                # key size on self-signed.pythontest.net.
                if re.search(r'(?i)key.too.weak', ssl_err_str):
                    raise unittest.SkipTest(
                        f'Got {ssl_err_str} trying to connect '
                        f'to {selfsigned_pythontestdotnet}. '
                        'See https://bugs.python.org/issue36816.')
                raise
            server_string = resp.getheader('server')
            resp.close()
            h.close()
            self.assertIn('nginx', server_string)

    def test_networked_bad_cert(self):
        # We feed a "CA" cert that is unrelated to the server's cert
        import ssl
        support.requires('network')
        with support.transient_internet('self-signed.pythontest.net'):
            context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
            context.load_verify_locations(CERT_localhost)
            h = client.HTTPSConnection('self-signed.pythontest.net', 443, context=context)
            with self.assertRaises(ssl.SSLError) as exc_info:
                h.request('GET', '/')
            self.assertEqual(exc_info.exception.reason, 'CERTIFICATE_VERIFY_FAILED')

    def test_local_unknown_cert(self):
        # The custom cert isn't known to the default trust bundle
        import ssl
        server = self.make_server(CERT_localhost)
        h = client.HTTPSConnection('localhost', server.port)
        with self.assertRaises(ssl.SSLError) as exc_info:
            h.request('GET', '/')
        self.assertEqual(exc_info.exception.reason, 'CERTIFICATE_VERIFY_FAILED')

    def test_local_good_hostname(self):
        # The (valid) cert validates the HTTP hostname
        import ssl
        server = self.make_server(CERT_localhost)
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        context.load_verify_locations(CERT_localhost)
        h = client.HTTPSConnection('localhost', server.port, context=context)
        self.addCleanup(h.close)
        h.request('GET', '/nonexistent')
        resp = h.getresponse()
        self.addCleanup(resp.close)
        self.assertEqual(resp.status, 404)

    def test_local_bad_hostname(self):
        # The (valid) cert doesn't validate the HTTP hostname
        import ssl
        server = self.make_server(CERT_fakehostname)
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        context.load_verify_locations(CERT_fakehostname)
        h = client.HTTPSConnection('localhost', server.port, context=context)
        with self.assertRaises(ssl.CertificateError):
            h.request('GET', '/')
        # Same with explicit check_hostname=True
        with support.check_warnings(('', DeprecationWarning)):
            h = client.HTTPSConnection('localhost', server.port,
                                       context=context, check_hostname=True)
        with self.assertRaises(ssl.CertificateError):
            h.request('GET', '/')
        # With check_hostname=False, the mismatching is ignored
        context.check_hostname = False
        with support.check_warnings(('', DeprecationWarning)):
            h = client.HTTPSConnection('localhost', server.port,
                                       context=context, check_hostname=False)
        h.request('GET', '/nonexistent')
        resp = h.getresponse()
        resp.close()
        h.close()
        self.assertEqual(resp.status, 404)
        # The context's check_hostname setting is used if one isn't passed to
        # HTTPSConnection.
        context.check_hostname = False
        h = client.HTTPSConnection('localhost', server.port, context=context)
        h.request('GET', '/nonexistent')
        resp = h.getresponse()
        self.assertEqual(resp.status, 404)
        resp.close()
        h.close()
        # Passing check_hostname to HTTPSConnection should override the
        # context's setting.
        with support.check_warnings(('', DeprecationWarning)):
            h = client.HTTPSConnection('localhost', server.port,
                                       context=context, check_hostname=True)
        with self.assertRaises(ssl.CertificateError):
            h.request('GET', '/')

    @unittest.skipIf(not hasattr(client, 'HTTPSConnection'),
                     'http.client.HTTPSConnection not available')
    def test_host_port(self):
        # Check invalid host_port

        for hp in ("www.python.org:abc", "user:password@www.python.org"):
            self.assertRaises(client.InvalidURL, client.HTTPSConnection, hp)

        for hp, h, p in (("[fe80::207:e9ff:fe9b]:8000",
                          "fe80::207:e9ff:fe9b", 8000),
                         ("www.python.org:443", "www.python.org", 443),
                         ("www.python.org:", "www.python.org", 443),
                         ("www.python.org", "www.python.org", 443),
                         ("[fe80::207:e9ff:fe9b]", "fe80::207:e9ff:fe9b", 443),
                         ("[fe80::207:e9ff:fe9b]:", "fe80::207:e9ff:fe9b",
                             443)):
            c = client.HTTPSConnection(hp)
            self.assertEqual(h, c.host)
            self.assertEqual(p, c.port)

    def test_tls13_pha(self):
        import ssl
        if not ssl.HAS_TLSv1_3:
            self.skipTest('TLS 1.3 support required')
        # just check status of PHA flag
        h = client.HTTPSConnection('localhost', 443)
        self.assertTrue(h._context.post_handshake_auth)

        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        self.assertFalse(context.post_handshake_auth)
        h = client.HTTPSConnection('localhost', 443, context=context)
        self.assertIs(h._context, context)
        self.assertFalse(h._context.post_handshake_auth)

        with warnings.catch_warnings():
            warnings.filterwarnings('ignore', 'key_file, cert_file and check_hostname are deprecated',
                                    DeprecationWarning)
            h = client.HTTPSConnection('localhost', 443, context=context,
                                       cert_file=CERT_localhost)
        self.assertTrue(h._context.post_handshake_auth)


class RequestBodyTest(TestCase):
    """Test cases where a request includes a message body."""

    def setUp(self):
        self.conn = client.HTTPConnection('example.com')
        self.conn.sock = self.sock = FakeSocket("")
        self.conn.sock = self.sock

    def get_headers_and_fp(self):
        f = io.BytesIO(self.sock.data)
        f.readline()  # read the request line
        message = client.parse_headers(f)
        return message, f

    def test_list_body(self):
        # Note that no content-length is automatically calculated for
        # an iterable.  The request will fall back to send chunked
        # transfer encoding.
        cases = (
            ([b'foo', b'bar'], b'3\r\nfoo\r\n3\r\nbar\r\n0\r\n\r\n'),
            ((b'foo', b'bar'), b'3\r\nfoo\r\n3\r\nbar\r\n0\r\n\r\n'),
        )
        for body, expected in cases:
            with self.subTest(body):
                self.conn = client.HTTPConnection('example.com')
                self.conn.sock = self.sock = FakeSocket('')

                self.conn.request('PUT', '/url', body)
                msg, f = self.get_headers_and_fp()
                self.assertNotIn('Content-Type', msg)
                self.assertNotIn('Content-Length', msg)
                self.assertEqual(msg.get('Transfer-Encoding'), 'chunked')
                self.assertEqual(expected, f.read())

    def test_manual_content_length(self):
        # Set an incorrect content-length so that we can verify that
        # it will not be over-ridden by the library.
        self.conn.request("PUT", "/url", "body",
                          {"Content-Length": "42"})
        message, f = self.get_headers_and_fp()
        self.assertEqual("42", message.get("content-length"))
        self.assertEqual(4, len(f.read()))

    def test_ascii_body(self):
        self.conn.request("PUT", "/url", "body")
        message, f = self.get_headers_and_fp()
        self.assertEqual("text/plain", message.get_content_type())
        self.assertIsNone(message.get_charset())
        self.assertEqual("4", message.get("content-length"))
        self.assertEqual(b'body', f.read())

    def test_latin1_body(self):
        self.conn.request("PUT", "/url", "body\xc1")
        message, f = self.get_headers_and_fp()
        self.assertEqual("text/plain", message.get_content_type())
        self.assertIsNone(message.get_charset())
        self.assertEqual("5", message.get("content-length"))
        self.assertEqual(b'body\xc1', f.read())

    def test_bytes_body(self):
        self.conn.request("PUT", "/url", b"body\xc1")
        message, f = self.get_headers_and_fp()
        self.assertEqual("text/plain", message.get_content_type())
        self.assertIsNone(message.get_charset())
        self.assertEqual("5", message.get("content-length"))
        self.assertEqual(b'body\xc1', f.read())

    def test_text_file_body(self):
        self.addCleanup(support.unlink, support.TESTFN)
        with open(support.TESTFN, "w") as f:
            f.write("body")
        with open(support.TESTFN) as f:
            self.conn.request("PUT", "/url", f)
            message, f = self.get_headers_and_fp()
            self.assertEqual("text/plain", message.get_content_type())
            self.assertIsNone(message.get_charset())
            # No content-length will be determined for files; the body
            # will be sent using chunked transfer encoding instead.
            self.assertIsNone(message.get("content-length"))
            self.assertEqual("chunked", message.get("transfer-encoding"))
            self.assertEqual(b'4\r\nbody\r\n0\r\n\r\n', f.read())

    def test_binary_file_body(self):
        self.addCleanup(support.unlink, support.TESTFN)
        with open(support.TESTFN, "wb") as f:
            f.write(b"body\xc1")
        with open(support.TESTFN, "rb") as f:
            self.conn.request("PUT", "/url", f)
            message, f = self.get_headers_and_fp()
            self.assertEqual("text/plain", message.get_content_type())
            self.assertIsNone(message.get_charset())
            self.assertEqual("chunked", message.get("Transfer-Encoding"))
            self.assertNotIn("Content-Length", message)
            self.assertEqual(b'5\r\nbody\xc1\r\n0\r\n\r\n', f.read())


class HTTPResponseTest(TestCase):

    def setUp(self):
        body = "HTTP/1.1 200 Ok\r\nMy-Header: first-value\r\nMy-Header: \
                second-value\r\n\r\nText"
        sock = FakeSocket(body)
        self.resp = client.HTTPResponse(sock)
        self.resp.begin()

    def test_getting_header(self):
        header = self.resp.getheader('My-Header')
        self.assertEqual(header, 'first-value, second-value')

        header = self.resp.getheader('My-Header', 'some default')
        self.assertEqual(header, 'first-value, second-value')

    def test_getting_nonexistent_header_with_string_default(self):
        header = self.resp.getheader('No-Such-Header', 'default-value')
        self.assertEqual(header, 'default-value')

    def test_getting_nonexistent_header_with_iterable_default(self):
        header = self.resp.getheader('No-Such-Header', ['default', 'values'])
        self.assertEqual(header, 'default, values')

        header = self.resp.getheader('No-Such-Header', ('default', 'values'))
        self.assertEqual(header, 'default, values')

    def test_getting_nonexistent_header_without_default(self):
        header = self.resp.getheader('No-Such-Header')
        self.assertEqual(header, None)

    def test_getting_header_defaultint(self):
        header = self.resp.getheader('No-Such-Header',default=42)
        self.assertEqual(header, 42)

class TunnelTests(TestCase):
    def setUp(self):
        response_text = (
            'HTTP/1.0 200 OK\r\n\r\n' # Reply to CONNECT
            'HTTP/1.1 200 OK\r\n' # Reply to HEAD
            'Content-Length: 42\r\n\r\n'
        )
        self.host = 'proxy.com'
        self.conn = client.HTTPConnection(self.host)
        self.conn._create_connection = self._create_connection(response_text)

    def tearDown(self):
        self.conn.close()

    def _create_connection(self, response_text):
        def create_connection(address, timeout=None, source_address=None):
            return FakeSocket(response_text, host=address[0], port=address[1])
        return create_connection

    def test_set_tunnel_host_port_headers(self):
        tunnel_host = 'destination.com'
        tunnel_port = 8888
        tunnel_headers = {'User-Agent': 'Mozilla/5.0 (compatible, MSIE 11)'}
        self.conn.set_tunnel(tunnel_host, port=tunnel_port,
                             headers=tunnel_headers)
        self.conn.request('HEAD', '/', '')
        self.assertEqual(self.conn.sock.host, self.host)
        self.assertEqual(self.conn.sock.port, client.HTTP_PORT)
        self.assertEqual(self.conn._tunnel_host, tunnel_host)
        self.assertEqual(self.conn._tunnel_port, tunnel_port)
        self.assertEqual(self.conn._tunnel_headers, tunnel_headers)

    def test_disallow_set_tunnel_after_connect(self):
        # Once connected, we shouldn't be able to tunnel anymore
        self.conn.connect()
        self.assertRaises(RuntimeError, self.conn.set_tunnel,
                          'destination.com')

    def test_connect_with_tunnel(self):
        self.conn.set_tunnel('destination.com')
        self.conn.request('HEAD', '/', '')
        self.assertEqual(self.conn.sock.host, self.host)
        self.assertEqual(self.conn.sock.port, client.HTTP_PORT)
        self.assertIn(b'CONNECT destination.com', self.conn.sock.data)
        # issue22095
        self.assertNotIn(b'Host: destination.com:None', self.conn.sock.data)
        self.assertIn(b'Host: destination.com', self.conn.sock.data)

        # This test should be removed when CONNECT gets the HTTP/1.1 blessing
        self.assertNotIn(b'Host: proxy.com', self.conn.sock.data)

    def test_connect_put_request(self):
        self.conn.set_tunnel('destination.com')
        self.conn.request('PUT', '/', '')
        self.assertEqual(self.conn.sock.host, self.host)
        self.assertEqual(self.conn.sock.port, client.HTTP_PORT)
        self.assertIn(b'CONNECT destination.com', self.conn.sock.data)
        self.assertIn(b'Host: destination.com', self.conn.sock.data)

    def test_tunnel_debuglog(self):
        expected_header = 'X-Dummy: 1'
        response_text = 'HTTP/1.0 200 OK\r\n{}\r\n\r\n'.format(expected_header)

        self.conn.set_debuglevel(1)
        self.conn._create_connection = self._create_connection(response_text)
        self.conn.set_tunnel('destination.com')

        with support.captured_stdout() as output:
            self.conn.request('PUT', '/', '')
        lines = output.getvalue().splitlines()
        self.assertIn('header: {}'.format(expected_header), lines)


if __name__ == '__main__':
    unittest.main(verbosity=2)