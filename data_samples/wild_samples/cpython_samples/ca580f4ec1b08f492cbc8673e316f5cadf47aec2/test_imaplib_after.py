        with self.reaped_pair(AuthHandler) as (server, client):
            self.assertTrue('AUTH=CRAM-MD5' in client.capabilities)
            ret, data = client.login_cram_md5("tim", b"tanstaaftanstaaf")
            self.assertEqual(ret, "OK")


    def test_linetoolong(self):
        class TooLongHandler(SimpleIMAPHandler):
            def handle(self):
                # Send a very long response line
                self.wfile.write(b'* OK ' + imaplib._MAXLINE*b'x' + b'\r\n')

        with self.reaped_server(TooLongHandler) as server:
            self.assertRaises(imaplib.IMAP4.error,
                              self.imap_class, *server.server_address)


class ThreadedNetworkedTests(BaseThreadedNetworkedTests):

    server_class = socketserver.TCPServer
    imap_class = imaplib.IMAP4


@unittest.skipUnless(ssl, "SSL not available")
class ThreadedNetworkedTestsSSL(BaseThreadedNetworkedTests):

    server_class = SecureTCPServer
    imap_class = IMAP4_SSL


class RemoteIMAPTest(unittest.TestCase):
    host = 'cyrus.andrew.cmu.edu'
    port = 143
    username = 'anonymous'
    password = 'pass'
    imap_class = imaplib.IMAP4

    def setUp(self):
        with transient_internet(self.host):
            self.server = self.imap_class(self.host, self.port)

    def tearDown(self):
        if self.server is not None:
            with transient_internet(self.host):
                self.server.logout()

    def test_logincapa(self):
        with transient_internet(self.host):
            for cap in self.server.capabilities:
                self.assertIsInstance(cap, str)
            self.assertIn('LOGINDISABLED', self.server.capabilities)
            self.assertIn('AUTH=ANONYMOUS', self.server.capabilities)
            rs = self.server.login(self.username, self.password)
            self.assertEqual(rs[0], 'OK')

    def test_logout(self):
        with transient_internet(self.host):
            rs = self.server.logout()
            self.server = None
            self.assertEqual(rs[0], 'BYE')


@unittest.skipUnless(ssl, "SSL not available")
class RemoteIMAP_STARTTLSTest(RemoteIMAPTest):

    def setUp(self):
        super().setUp()
        with transient_internet(self.host):
            rs = self.server.starttls()
            self.assertEqual(rs[0], 'OK')

    def test_logincapa(self):
        for cap in self.server.capabilities:
            self.assertIsInstance(cap, str)
        self.assertNotIn('LOGINDISABLED', self.server.capabilities)


@unittest.skipUnless(ssl, "SSL not available")
class RemoteIMAP_SSLTest(RemoteIMAPTest):
    port = 993
    imap_class = IMAP4_SSL

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def create_ssl_context(self):
        ssl_context = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
        ssl_context.load_cert_chain(CERTFILE)
        return ssl_context

    def check_logincapa(self, server):
        try:
            for cap in server.capabilities:
                self.assertIsInstance(cap, str)
            self.assertNotIn('LOGINDISABLED', server.capabilities)
            self.assertIn('AUTH=PLAIN', server.capabilities)
            rs = server.login(self.username, self.password)
            self.assertEqual(rs[0], 'OK')
        finally:
            server.logout()

    def test_logincapa(self):
        with transient_internet(self.host):
            _server = self.imap_class(self.host, self.port)
            self.check_logincapa(_server)

    def test_logincapa_with_client_certfile(self):
        with transient_internet(self.host):
            _server = self.imap_class(self.host, self.port, certfile=CERTFILE)
            self.check_logincapa(_server)

    def test_logincapa_with_client_ssl_context(self):
        with transient_internet(self.host):
            _server = self.imap_class(self.host, self.port, ssl_context=self.create_ssl_context())
            self.check_logincapa(_server)

    def test_logout(self):
        with transient_internet(self.host):
            _server = self.imap_class(self.host, self.port)
            rs = _server.logout()
            self.assertEqual(rs[0], 'BYE')

    def test_ssl_context_certfile_exclusive(self):
        with transient_internet(self.host):
            self.assertRaises(ValueError, self.imap_class, self.host, self.port,
                              certfile=CERTFILE, ssl_context=self.create_ssl_context())

    def test_ssl_context_keyfile_exclusive(self):
        with transient_internet(self.host):
            self.assertRaises(ValueError, self.imap_class, self.host, self.port,
                              keyfile=CERTFILE, ssl_context=self.create_ssl_context())


def load_tests(*args):
    tests = [TestImaplib]

    if support.is_resource_enabled('network'):
        if ssl:
            global CERTFILE
            CERTFILE = os.path.join(os.path.dirname(__file__) or os.curdir,
                                    "keycert.pem")
            if not os.path.exists(CERTFILE):
                raise support.TestFailed("Can't read certificate files!")
        tests.extend([
            ThreadedNetworkedTests, ThreadedNetworkedTestsSSL,
            RemoteIMAPTest, RemoteIMAP_SSLTest, RemoteIMAP_STARTTLSTest,
        ])

    return unittest.TestSuite([unittest.makeSuite(test) for test in tests])


if __name__ == "__main__":
    support.use_resources = ['network']
    unittest.main()