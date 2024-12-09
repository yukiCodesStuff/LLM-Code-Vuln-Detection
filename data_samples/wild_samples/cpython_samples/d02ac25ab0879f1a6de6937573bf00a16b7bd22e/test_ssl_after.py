        with self.assertRaises(ValueError):
            ctx.wrap_bio(ssl.MemoryBIO(), ssl.MemoryBIO(),
                         server_hostname=".example.org")
        with self.assertRaises(TypeError):
            ctx.wrap_bio(ssl.MemoryBIO(), ssl.MemoryBIO(),
                         server_hostname="example.org\x00evil.com")


class MemoryBIOTests(unittest.TestCase):

    def test_read_write(self):
        bio = ssl.MemoryBIO()
        bio.write(b'foo')
        self.assertEqual(bio.read(), b'foo')
        self.assertEqual(bio.read(), b'')
        bio.write(b'foo')
        bio.write(b'bar')
        self.assertEqual(bio.read(), b'foobar')
        self.assertEqual(bio.read(), b'')
        bio.write(b'baz')
        self.assertEqual(bio.read(2), b'ba')
        self.assertEqual(bio.read(1), b'z')
        self.assertEqual(bio.read(1), b'')

    def test_eof(self):
        bio = ssl.MemoryBIO()
        self.assertFalse(bio.eof)
        self.assertEqual(bio.read(), b'')
        self.assertFalse(bio.eof)
        bio.write(b'foo')
        self.assertFalse(bio.eof)
        bio.write_eof()
        self.assertFalse(bio.eof)
        self.assertEqual(bio.read(2), b'fo')
        self.assertFalse(bio.eof)
        self.assertEqual(bio.read(1), b'o')
        self.assertTrue(bio.eof)
        self.assertEqual(bio.read(), b'')
        self.assertTrue(bio.eof)

    def test_pending(self):
        bio = ssl.MemoryBIO()
        self.assertEqual(bio.pending, 0)
        bio.write(b'foo')
        self.assertEqual(bio.pending, 3)
        for i in range(3):
            bio.read(1)
            self.assertEqual(bio.pending, 3-i-1)
        for i in range(3):
            bio.write(b'x')
            self.assertEqual(bio.pending, i+1)
        bio.read()
        self.assertEqual(bio.pending, 0)

    def test_buffer_types(self):
        bio = ssl.MemoryBIO()
        bio.write(b'foo')
        self.assertEqual(bio.read(), b'foo')
        bio.write(bytearray(b'bar'))
        self.assertEqual(bio.read(), b'bar')
        bio.write(memoryview(b'baz'))
        self.assertEqual(bio.read(), b'baz')

    def test_error_types(self):
        bio = ssl.MemoryBIO()
        self.assertRaises(TypeError, bio.write, 'foo')
        self.assertRaises(TypeError, bio.write, None)
        self.assertRaises(TypeError, bio.write, True)
        self.assertRaises(TypeError, bio.write, 1)


class SSLObjectTests(unittest.TestCase):
    def test_private_init(self):
        bio = ssl.MemoryBIO()
        with self.assertRaisesRegex(TypeError, "public constructor"):
            ssl.SSLObject(bio, bio)


class SimpleBackgroundTests(unittest.TestCase):
    """Tests that connect to a simple server running in the background"""

    def setUp(self):
        server = ThreadedEchoServer(SIGNED_CERTFILE)
        self.server_addr = (HOST, server.port)
        server.__enter__()
        self.addCleanup(server.__exit__, None, None, None)

    def test_connect(self):
        with test_wrap_socket(socket.socket(socket.AF_INET),
                            cert_reqs=ssl.CERT_NONE) as s:
            s.connect(self.server_addr)
            self.assertEqual({}, s.getpeercert())
            self.assertFalse(s.server_side)

        # this should succeed because we specify the root cert
        with test_wrap_socket(socket.socket(socket.AF_INET),
                            cert_reqs=ssl.CERT_REQUIRED,
                            ca_certs=SIGNING_CA) as s:
            s.connect(self.server_addr)
            self.assertTrue(s.getpeercert())
            self.assertFalse(s.server_side)

    def test_connect_fail(self):
        # This should fail because we have no verification certs. Connection
        # failure crashes ThreadedEchoServer, so run this in an independent
        # test method.
        s = test_wrap_socket(socket.socket(socket.AF_INET),
                            cert_reqs=ssl.CERT_REQUIRED)
        self.addCleanup(s.close)
        self.assertRaisesRegex(ssl.SSLError, "certificate verify failed",
                               s.connect, self.server_addr)

    def test_connect_ex(self):
        # Issue #11326: check connect_ex() implementation
        s = test_wrap_socket(socket.socket(socket.AF_INET),
                            cert_reqs=ssl.CERT_REQUIRED,
                            ca_certs=SIGNING_CA)
        self.addCleanup(s.close)
        self.assertEqual(0, s.connect_ex(self.server_addr))
        self.assertTrue(s.getpeercert())

    def test_non_blocking_connect_ex(self):
        # Issue #11326: non-blocking connect_ex() should allow handshake
        # to proceed after the socket gets ready.
        s = test_wrap_socket(socket.socket(socket.AF_INET),
                            cert_reqs=ssl.CERT_REQUIRED,
                            ca_certs=SIGNING_CA,
                            do_handshake_on_connect=False)
        self.addCleanup(s.close)
        s.setblocking(False)
        rc = s.connect_ex(self.server_addr)
        # EWOULDBLOCK under Windows, EINPROGRESS elsewhere
        self.assertIn(rc, (0, errno.EINPROGRESS, errno.EWOULDBLOCK))
        # Wait for connect to finish
        select.select([], [s], [], 5.0)
        # Non-blocking handshake
        while True:
            try:
                s.do_handshake()
                break
            except ssl.SSLWantReadError:
                select.select([s], [], [], 5.0)
            except ssl.SSLWantWriteError:
                select.select([], [s], [], 5.0)
        # SSL established
        self.assertTrue(s.getpeercert())

    def test_connect_with_context(self):
        # Same as test_connect, but with a separately created context
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS)
        with ctx.wrap_socket(socket.socket(socket.AF_INET)) as s:
            s.connect(self.server_addr)
            self.assertEqual({}, s.getpeercert())
        # Same with a server hostname
        with ctx.wrap_socket(socket.socket(socket.AF_INET),
                            server_hostname="dummy") as s:
            s.connect(self.server_addr)
        ctx.verify_mode = ssl.CERT_REQUIRED
        # This should succeed because we specify the root cert
        ctx.load_verify_locations(SIGNING_CA)
        with ctx.wrap_socket(socket.socket(socket.AF_INET)) as s:
            s.connect(self.server_addr)
            cert = s.getpeercert()
            self.assertTrue(cert)

    def test_connect_with_context_fail(self):
        # This should fail because we have no verification certs. Connection
        # failure crashes ThreadedEchoServer, so run this in an independent
        # test method.
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS)
        ctx.verify_mode = ssl.CERT_REQUIRED
        s = ctx.wrap_socket(socket.socket(socket.AF_INET))
        self.addCleanup(s.close)
        self.assertRaisesRegex(ssl.SSLError, "certificate verify failed",
                                s.connect, self.server_addr)

    def test_connect_capath(self):
        # Verify server certificates using the `capath` argument
        # NOTE: the subject hashing algorithm has been changed between
        # OpenSSL 0.9.8n and 1.0.0, as a result the capath directory must
        # contain both versions of each certificate (same content, different
        # filename) for this test to be portable across OpenSSL releases.
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS)
        ctx.verify_mode = ssl.CERT_REQUIRED
        ctx.load_verify_locations(capath=CAPATH)
        with ctx.wrap_socket(socket.socket(socket.AF_INET)) as s:
            s.connect(self.server_addr)
            cert = s.getpeercert()
            self.assertTrue(cert)
        # Same with a bytes `capath` argument
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS)
        ctx.verify_mode = ssl.CERT_REQUIRED
        ctx.load_verify_locations(capath=BYTES_CAPATH)
        with ctx.wrap_socket(socket.socket(socket.AF_INET)) as s:
            s.connect(self.server_addr)
            cert = s.getpeercert()
            self.assertTrue(cert)

    def test_connect_cadata(self):
        with open(SIGNING_CA) as f:
            pem = f.read()
        der = ssl.PEM_cert_to_DER_cert(pem)
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS)
        ctx.verify_mode = ssl.CERT_REQUIRED
        # TODO: fix TLSv1.3 support
        ctx.options |= ssl.OP_NO_TLSv1_3
        ctx.load_verify_locations(cadata=pem)
        with ctx.wrap_socket(socket.socket(socket.AF_INET)) as s:
            s.connect(self.server_addr)
            cert = s.getpeercert()
            self.assertTrue(cert)

        # same with DER
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS)
        ctx.verify_mode = ssl.CERT_REQUIRED
        # TODO: fix TLSv1.3 support
        ctx.options |= ssl.OP_NO_TLSv1_3
        ctx.load_verify_locations(cadata=der)
        with ctx.wrap_socket(socket.socket(socket.AF_INET)) as s:
            s.connect(self.server_addr)
            cert = s.getpeercert()
            self.assertTrue(cert)

    @unittest.skipIf(os.name == "nt", "Can't use a socket as a file under Windows")
    def test_makefile_close(self):
        # Issue #5238: creating a file-like object with makefile() shouldn't
        # delay closing the underlying "real socket" (here tested with its
        # file descriptor, hence skipping the test under Windows).
        ss = test_wrap_socket(socket.socket(socket.AF_INET))
        ss.connect(self.server_addr)
        fd = ss.fileno()
        f = ss.makefile()
        f.close()
        # The fd is still open
        os.read(fd, 0)
        # Closing the SSL socket should close the fd too
        ss.close()
        gc.collect()
        with self.assertRaises(OSError) as e:
            os.read(fd, 0)
        self.assertEqual(e.exception.errno, errno.EBADF)

    def test_non_blocking_handshake(self):
        s = socket.socket(socket.AF_INET)
        s.connect(self.server_addr)
        s.setblocking(False)
        s = test_wrap_socket(s,
                            cert_reqs=ssl.CERT_NONE,
                            do_handshake_on_connect=False)
        self.addCleanup(s.close)
        count = 0
        while True:
            try:
                count += 1
                s.do_handshake()
                break
            except ssl.SSLWantReadError:
                select.select([s], [], [])
            except ssl.SSLWantWriteError:
                select.select([], [s], [])
        if support.verbose:
            sys.stdout.write("\nNeeded %d calls to do_handshake() to establish session.\n" % count)

    def test_get_server_certificate(self):
        _test_get_server_certificate(self, *self.server_addr, cert=SIGNING_CA)

    def test_get_server_certificate_fail(self):
        # Connection failure crashes ThreadedEchoServer, so run this in an
        # independent test method
        _test_get_server_certificate_fail(self, *self.server_addr)

    def test_ciphers(self):
        with test_wrap_socket(socket.socket(socket.AF_INET),
                             cert_reqs=ssl.CERT_NONE, ciphers="ALL") as s:
            s.connect(self.server_addr)
        with test_wrap_socket(socket.socket(socket.AF_INET),
                             cert_reqs=ssl.CERT_NONE, ciphers="DEFAULT") as s:
            s.connect(self.server_addr)
        # Error checking can happen at instantiation or when connecting
        with self.assertRaisesRegex(ssl.SSLError, "No cipher can be selected"):
            with socket.socket(socket.AF_INET) as sock:
                s = test_wrap_socket(sock,
                                    cert_reqs=ssl.CERT_NONE, ciphers="^$:,;?*'dorothyx")
                s.connect(self.server_addr)

    def test_get_ca_certs_capath(self):
        # capath certs are loaded on request
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        ctx.load_verify_locations(capath=CAPATH)
        self.assertEqual(ctx.get_ca_certs(), [])
        with ctx.wrap_socket(socket.socket(socket.AF_INET),
                             server_hostname='localhost') as s:
            s.connect(self.server_addr)
            cert = s.getpeercert()
            self.assertTrue(cert)
        self.assertEqual(len(ctx.get_ca_certs()), 1)

    @needs_sni
    def test_context_setget(self):
        # Check that the context of a connected socket can be replaced.
        ctx1 = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        ctx1.load_verify_locations(capath=CAPATH)
        ctx2 = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        ctx2.load_verify_locations(capath=CAPATH)
        s = socket.socket(socket.AF_INET)
        with ctx1.wrap_socket(s, server_hostname='localhost') as ss:
            ss.connect(self.server_addr)
            self.assertIs(ss.context, ctx1)
            self.assertIs(ss._sslobj.context, ctx1)
            ss.context = ctx2
            self.assertIs(ss.context, ctx2)
            self.assertIs(ss._sslobj.context, ctx2)

    def ssl_io_loop(self, sock, incoming, outgoing, func, *args, **kwargs):
        # A simple IO loop. Call func(*args) depending on the error we get
        # (WANT_READ or WANT_WRITE) move data between the socket and the BIOs.
        timeout = kwargs.get('timeout', 10)
        deadline = time.monotonic() + timeout
        count = 0
        while True:
            if time.monotonic() > deadline:
                self.fail("timeout")
            errno = None
            count += 1
            try:
                ret = func(*args)
            except ssl.SSLError as e:
                if e.errno not in (ssl.SSL_ERROR_WANT_READ,
                                   ssl.SSL_ERROR_WANT_WRITE):
                    raise
                errno = e.errno
            # Get any data from the outgoing BIO irrespective of any error, and
            # send it to the socket.
            buf = outgoing.read()
            sock.sendall(buf)
            # If there's no error, we're done. For WANT_READ, we need to get
            # data from the socket and put it in the incoming BIO.
            if errno is None:
                break
            elif errno == ssl.SSL_ERROR_WANT_READ:
                buf = sock.recv(32768)
                if buf:
                    incoming.write(buf)
                else:
                    incoming.write_eof()
        if support.verbose:
            sys.stdout.write("Needed %d calls to complete %s().\n"
                             % (count, func.__name__))
        return ret

    def test_bio_handshake(self):
        sock = socket.socket(socket.AF_INET)
        self.addCleanup(sock.close)
        sock.connect(self.server_addr)
        incoming = ssl.MemoryBIO()
        outgoing = ssl.MemoryBIO()
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        self.assertTrue(ctx.check_hostname)
        self.assertEqual(ctx.verify_mode, ssl.CERT_REQUIRED)
        ctx.load_verify_locations(SIGNING_CA)
        sslobj = ctx.wrap_bio(incoming, outgoing, False,
                              SIGNED_CERTFILE_HOSTNAME)
        self.assertIs(sslobj._sslobj.owner, sslobj)
        self.assertIsNone(sslobj.cipher())
        self.assertIsNone(sslobj.version())
        self.assertIsNotNone(sslobj.shared_ciphers())
        self.assertRaises(ValueError, sslobj.getpeercert)
        if 'tls-unique' in ssl.CHANNEL_BINDING_TYPES:
            self.assertIsNone(sslobj.get_channel_binding('tls-unique'))
        self.ssl_io_loop(sock, incoming, outgoing, sslobj.do_handshake)
        self.assertTrue(sslobj.cipher())
        self.assertIsNotNone(sslobj.shared_ciphers())
        self.assertIsNotNone(sslobj.version())
        self.assertTrue(sslobj.getpeercert())
        if 'tls-unique' in ssl.CHANNEL_BINDING_TYPES:
            self.assertTrue(sslobj.get_channel_binding('tls-unique'))
        try:
            self.ssl_io_loop(sock, incoming, outgoing, sslobj.unwrap)
        except ssl.SSLSyscallError:
            # If the server shuts down the TCP connection without sending a
            # secure shutdown message, this is reported as SSL_ERROR_SYSCALL
            pass
        self.assertRaises(ssl.SSLError, sslobj.write, b'foo')

    def test_bio_read_write_data(self):
        sock = socket.socket(socket.AF_INET)
        self.addCleanup(sock.close)
        sock.connect(self.server_addr)
        incoming = ssl.MemoryBIO()
        outgoing = ssl.MemoryBIO()
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS)
        ctx.verify_mode = ssl.CERT_NONE
        sslobj = ctx.wrap_bio(incoming, outgoing, False)
        self.ssl_io_loop(sock, incoming, outgoing, sslobj.do_handshake)
        req = b'FOO\n'
        self.ssl_io_loop(sock, incoming, outgoing, sslobj.write, req)
        buf = self.ssl_io_loop(sock, incoming, outgoing, sslobj.read, 1024)
        self.assertEqual(buf, b'foo\n')
        self.ssl_io_loop(sock, incoming, outgoing, sslobj.unwrap)


class NetworkedTests(unittest.TestCase):

    def test_timeout_connect_ex(self):
        # Issue #12065: on a timeout, connect_ex() should return the original
        # errno (mimicking the behaviour of non-SSL sockets).
        with support.transient_internet(REMOTE_HOST):
            s = test_wrap_socket(socket.socket(socket.AF_INET),
                                cert_reqs=ssl.CERT_REQUIRED,
                                do_handshake_on_connect=False)
            self.addCleanup(s.close)
            s.settimeout(0.0000001)
            rc = s.connect_ex((REMOTE_HOST, 443))
            if rc == 0:
                self.skipTest("REMOTE_HOST responded too quickly")
            self.assertIn(rc, (errno.EAGAIN, errno.EWOULDBLOCK))

    @unittest.skipUnless(support.IPV6_ENABLED, 'Needs IPv6')
    def test_get_server_certificate_ipv6(self):
        with support.transient_internet('ipv6.google.com'):
            _test_get_server_certificate(self, 'ipv6.google.com', 443)
            _test_get_server_certificate_fail(self, 'ipv6.google.com', 443)


def _test_get_server_certificate(test, host, port, cert=None):
    pem = ssl.get_server_certificate((host, port))
    if not pem:
        test.fail("No server certificate on %s:%s!" % (host, port))

    pem = ssl.get_server_certificate((host, port), ca_certs=cert)
    if not pem:
        test.fail("No server certificate on %s:%s!" % (host, port))
    if support.verbose:
        sys.stdout.write("\nVerified certificate for %s:%s is\n%s\n" % (host, port ,pem))

def _test_get_server_certificate_fail(test, host, port):
    try:
        pem = ssl.get_server_certificate((host, port), ca_certs=CERTFILE)
    except ssl.SSLError as x:
        #should fail
        if support.verbose:
            sys.stdout.write("%s\n" % x)
    else:
        test.fail("Got server certificate %s for %s:%s!" % (pem, host, port))


from test.ssl_servers import make_https_server

class ThreadedEchoServer(threading.Thread):

    class ConnectionHandler(threading.Thread):

        """A mildly complicated class, because we want it to work both
        with and without the SSL wrapper around the socket connection, so
        that we can test the STARTTLS functionality."""

        def __init__(self, server, connsock, addr):
            self.server = server
            self.running = False
            self.sock = connsock
            self.addr = addr
            self.sock.setblocking(1)
            self.sslconn = None
            threading.Thread.__init__(self)
            self.daemon = True

        def wrap_conn(self):
            try:
                self.sslconn = self.server.context.wrap_socket(
                    self.sock, server_side=True)
                self.server.selected_npn_protocols.append(self.sslconn.selected_npn_protocol())
                self.server.selected_alpn_protocols.append(self.sslconn.selected_alpn_protocol())
            except (ssl.SSLError, ConnectionResetError, OSError) as e:
                # We treat ConnectionResetError as though it were an
                # SSLError - OpenSSL on Ubuntu abruptly closes the
                # connection when asked to use an unsupported protocol.
                #
                # OSError may occur with wrong protocols, e.g. both
                # sides use PROTOCOL_TLS_SERVER.
                #
                # XXX Various errors can have happened here, for example
                # a mismatching protocol version, an invalid certificate,
                # or a low-level bug. This should be made more discriminating.
                #
                # bpo-31323: Store the exception as string to prevent
                # a reference leak: server -> conn_errors -> exception
                # -> traceback -> self (ConnectionHandler) -> server
                self.server.conn_errors.append(str(e))
                if self.server.chatty:
                    handle_error("\n server:  bad connection attempt from " + repr(self.addr) + ":\n")
                self.running = False
                self.server.stop()
                self.close()
                return False
            else:
                self.server.shared_ciphers.append(self.sslconn.shared_ciphers())
                if self.server.context.verify_mode == ssl.CERT_REQUIRED:
                    cert = self.sslconn.getpeercert()
                    if support.verbose and self.server.chatty:
                        sys.stdout.write(" client cert is " + pprint.pformat(cert) + "\n")
                    cert_binary = self.sslconn.getpeercert(True)
                    if support.verbose and self.server.chatty:
                        sys.stdout.write(" cert binary is " + str(len(cert_binary)) + " bytes\n")
                cipher = self.sslconn.cipher()
                if support.verbose and self.server.chatty:
                    sys.stdout.write(" server: connection cipher is now " + str(cipher) + "\n")
                    sys.stdout.write(" server: selected protocol is now "
                            + str(self.sslconn.selected_npn_protocol()) + "\n")
                return True

        def read(self):
            if self.sslconn:
                return self.sslconn.read()
            else:
                return self.sock.recv(1024)

        def write(self, bytes):
            if self.sslconn:
                return self.sslconn.write(bytes)
            else:
                return self.sock.send(bytes)

        def close(self):
            if self.sslconn:
                self.sslconn.close()
            else:
                self.sock.close()

        def run(self):
            self.running = True
            if not self.server.starttls_server:
                if not self.wrap_conn():
                    return
            while self.running:
                try:
                    msg = self.read()
                    stripped = msg.strip()
                    if not stripped:
                        # eof, so quit this handler
                        self.running = False
                        try:
                            self.sock = self.sslconn.unwrap()
                        except OSError:
                            # Many tests shut the TCP connection down
                            # without an SSL shutdown. This causes
                            # unwrap() to raise OSError with errno=0!
                            pass
                        else:
                            self.sslconn = None
                        self.close()
                    elif stripped == b'over':
                        if support.verbose and self.server.connectionchatty:
                            sys.stdout.write(" server: client closed connection\n")
                        self.close()
                        return
                    elif (self.server.starttls_server and
                          stripped == b'STARTTLS'):
                        if support.verbose and self.server.connectionchatty:
                            sys.stdout.write(" server: read STARTTLS from client, sending OK...\n")
                        self.write(b"OK\n")
                        if not self.wrap_conn():
                            return
                    elif (self.server.starttls_server and self.sslconn
                          and stripped == b'ENDTLS'):
                        if support.verbose and self.server.connectionchatty:
                            sys.stdout.write(" server: read ENDTLS from client, sending OK...\n")
                        self.write(b"OK\n")
                        self.sock = self.sslconn.unwrap()
                        self.sslconn = None
                        if support.verbose and self.server.connectionchatty:
                            sys.stdout.write(" server: connection is now unencrypted...\n")
                    elif stripped == b'CB tls-unique':
                        if support.verbose and self.server.connectionchatty:
                            sys.stdout.write(" server: read CB tls-unique from client, sending our CB data...\n")
                        data = self.sslconn.get_channel_binding("tls-unique")
                        self.write(repr(data).encode("us-ascii") + b"\n")
                    else:
                        if (support.verbose and
                            self.server.connectionchatty):
                            ctype = (self.sslconn and "encrypted") or "unencrypted"
                            sys.stdout.write(" server: read %r (%s), sending back %r (%s)...\n"
                                             % (msg, ctype, msg.lower(), ctype))
                        self.write(msg.lower())
                except OSError:
                    if self.server.chatty:
                        handle_error("Test server failure:\n")
                    self.close()
                    self.running = False
                    # normally, we'd just stop here, but for the test
                    # harness, we want to stop the server
                    self.server.stop()

    def __init__(self, certificate=None, ssl_version=None,
                 certreqs=None, cacerts=None,
                 chatty=True, connectionchatty=False, starttls_server=False,
                 npn_protocols=None, alpn_protocols=None,
                 ciphers=None, context=None):
        if context:
            self.context = context
        else:
            self.context = ssl.SSLContext(ssl_version
                                          if ssl_version is not None
                                          else ssl.PROTOCOL_TLS_SERVER)
            self.context.verify_mode = (certreqs if certreqs is not None
                                        else ssl.CERT_NONE)
            if cacerts:
                self.context.load_verify_locations(cacerts)
            if certificate:
                self.context.load_cert_chain(certificate)
            if npn_protocols:
                self.context.set_npn_protocols(npn_protocols)
            if alpn_protocols:
                self.context.set_alpn_protocols(alpn_protocols)
            if ciphers:
                self.context.set_ciphers(ciphers)
        self.chatty = chatty
        self.connectionchatty = connectionchatty
        self.starttls_server = starttls_server
        self.sock = socket.socket()
        self.port = support.bind_port(self.sock)
        self.flag = None
        self.active = False
        self.selected_npn_protocols = []
        self.selected_alpn_protocols = []
        self.shared_ciphers = []
        self.conn_errors = []
        threading.Thread.__init__(self)
        self.daemon = True

    def __enter__(self):
        self.start(threading.Event())
        self.flag.wait()
        return self

    def __exit__(self, *args):
        self.stop()
        self.join()

    def start(self, flag=None):
        self.flag = flag
        threading.Thread.start(self)

    def run(self):
        self.sock.settimeout(0.05)
        self.sock.listen()
        self.active = True
        if self.flag:
            # signal an event
            self.flag.set()
        while self.active:
            try:
                newconn, connaddr = self.sock.accept()
                if support.verbose and self.chatty:
                    sys.stdout.write(' server:  new connection from '
                                     + repr(connaddr) + '\n')
                handler = self.ConnectionHandler(self, newconn, connaddr)
                handler.start()
                handler.join()
            except socket.timeout:
                pass
            except KeyboardInterrupt:
                self.stop()
        self.sock.close()

    def stop(self):
        self.active = False

class AsyncoreEchoServer(threading.Thread):

    # this one's based on asyncore.dispatcher

    class EchoServer (asyncore.dispatcher):

        class ConnectionHandler(asyncore.dispatcher_with_send):

            def __init__(self, conn, certfile):
                self.socket = test_wrap_socket(conn, server_side=True,
                                              certfile=certfile,
                                              do_handshake_on_connect=False)
                asyncore.dispatcher_with_send.__init__(self, self.socket)
                self._ssl_accepting = True
                self._do_ssl_handshake()

            def readable(self):
                if isinstance(self.socket, ssl.SSLSocket):
                    while self.socket.pending() > 0:
                        self.handle_read_event()
                return True

            def _do_ssl_handshake(self):
                try:
                    self.socket.do_handshake()
                except (ssl.SSLWantReadError, ssl.SSLWantWriteError):
                    return
                except ssl.SSLEOFError:
                    return self.handle_close()
                except ssl.SSLError:
                    raise
                except OSError as err:
                    if err.args[0] == errno.ECONNABORTED:
                        return self.handle_close()
                else:
                    self._ssl_accepting = False

            def handle_read(self):
                if self._ssl_accepting:
                    self._do_ssl_handshake()
                else:
                    data = self.recv(1024)
                    if support.verbose:
                        sys.stdout.write(" server:  read %s from client\n" % repr(data))
                    if not data:
                        self.close()
                    else:
                        self.send(data.lower())

            def handle_close(self):
                self.close()
                if support.verbose:
                    sys.stdout.write(" server:  closed connection %s\n" % self.socket)

            def handle_error(self):
                raise

        def __init__(self, certfile):
            self.certfile = certfile
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.port = support.bind_port(sock, '')
            asyncore.dispatcher.__init__(self, sock)
            self.listen(5)

        def handle_accepted(self, sock_obj, addr):
            if support.verbose:
                sys.stdout.write(" server:  new connection from %s:%s\n" %addr)
            self.ConnectionHandler(sock_obj, self.certfile)

        def handle_error(self):
            raise

    def __init__(self, certfile):
        self.flag = None
        self.active = False
        self.server = self.EchoServer(certfile)
        self.port = self.server.port
        threading.Thread.__init__(self)
        self.daemon = True

    def __str__(self):
        return "<%s %s>" % (self.__class__.__name__, self.server)

    def __enter__(self):
        self.start(threading.Event())
        self.flag.wait()
        return self

    def __exit__(self, *args):
        if support.verbose:
            sys.stdout.write(" cleanup: stopping server.\n")
        self.stop()
        if support.verbose:
            sys.stdout.write(" cleanup: joining server thread.\n")
        self.join()
        if support.verbose:
            sys.stdout.write(" cleanup: successfully joined.\n")
        # make sure that ConnectionHandler is removed from socket_map
        asyncore.close_all(ignore_all=True)

    def start (self, flag=None):
        self.flag = flag
        threading.Thread.start(self)

    def run(self):
        self.active = True
        if self.flag:
            self.flag.set()
        while self.active:
            try:
                asyncore.loop(1)
            except:
                pass

    def stop(self):
        self.active = False
        self.server.close()

def server_params_test(client_context, server_context, indata=b"FOO\n",
                       chatty=True, connectionchatty=False, sni_name=None,
                       session=None):
    """
    Launch a server, connect a client to it and try various reads
    and writes.
    """
    stats = {}
    server = ThreadedEchoServer(context=server_context,
                                chatty=chatty,
                                connectionchatty=False)
    with server:
        with client_context.wrap_socket(socket.socket(),
                server_hostname=sni_name, session=session) as s:
            s.connect((HOST, server.port))
            for arg in [indata, bytearray(indata), memoryview(indata)]:
                if connectionchatty:
                    if support.verbose:
                        sys.stdout.write(
                            " client:  sending %r...\n" % indata)
                s.write(arg)
                outdata = s.read()
                if connectionchatty:
                    if support.verbose:
                        sys.stdout.write(" client:  read %r\n" % outdata)
                if outdata != indata.lower():
                    raise AssertionError(
                        "bad data <<%r>> (%d) received; expected <<%r>> (%d)\n"
                        % (outdata[:20], len(outdata),
                           indata[:20].lower(), len(indata)))
            s.write(b"over\n")
            if connectionchatty:
                if support.verbose:
                    sys.stdout.write(" client:  closing connection.\n")
            stats.update({
                'compression': s.compression(),
                'cipher': s.cipher(),
                'peercert': s.getpeercert(),
                'client_alpn_protocol': s.selected_alpn_protocol(),
                'client_npn_protocol': s.selected_npn_protocol(),
                'version': s.version(),
                'session_reused': s.session_reused,
                'session': s.session,
            })
            s.close()
        stats['server_alpn_protocols'] = server.selected_alpn_protocols
        stats['server_npn_protocols'] = server.selected_npn_protocols
        stats['server_shared_ciphers'] = server.shared_ciphers
    return stats

def try_protocol_combo(server_protocol, client_protocol, expect_success,
                       certsreqs=None, server_options=0, client_options=0):
    """
    Try to SSL-connect using *client_protocol* to *server_protocol*.
    If *expect_success* is true, assert that the connection succeeds,
    if it's false, assert that the connection fails.
    Also, if *expect_success* is a string, assert that it is the protocol
    version actually used by the connection.
    """
    if certsreqs is None:
        certsreqs = ssl.CERT_NONE
    certtype = {
        ssl.CERT_NONE: "CERT_NONE",
        ssl.CERT_OPTIONAL: "CERT_OPTIONAL",
        ssl.CERT_REQUIRED: "CERT_REQUIRED",
    }[certsreqs]
    if support.verbose:
        formatstr = (expect_success and " %s->%s %s\n") or " {%s->%s} %s\n"
        sys.stdout.write(formatstr %
                         (ssl.get_protocol_name(client_protocol),
                          ssl.get_protocol_name(server_protocol),
                          certtype))
    client_context = ssl.SSLContext(client_protocol)
    client_context.options |= client_options
    server_context = ssl.SSLContext(server_protocol)
    server_context.options |= server_options

    # NOTE: we must enable "ALL" ciphers on the client, otherwise an
    # SSLv23 client will send an SSLv3 hello (rather than SSLv2)
    # starting from OpenSSL 1.0.0 (see issue #8322).
    if client_context.protocol == ssl.PROTOCOL_TLS:
        client_context.set_ciphers("ALL")

    for ctx in (client_context, server_context):
        ctx.verify_mode = certsreqs
        ctx.load_cert_chain(SIGNED_CERTFILE)
        ctx.load_verify_locations(SIGNING_CA)
    try:
        stats = server_params_test(client_context, server_context,
                                   chatty=False, connectionchatty=False)
    # Protocol mismatch can result in either an SSLError, or a
    # "Connection reset by peer" error.
    except ssl.SSLError:
        if expect_success:
            raise
    except OSError as e:
        if expect_success or e.errno != errno.ECONNRESET:
            raise
    else:
        if not expect_success:
            raise AssertionError(
                "Client protocol %s succeeded with server protocol %s!"
                % (ssl.get_protocol_name(client_protocol),
                   ssl.get_protocol_name(server_protocol)))
        elif (expect_success is not True
              and expect_success != stats['version']):
            raise AssertionError("version mismatch: expected %r, got %r"
                                 % (expect_success, stats['version']))


class ThreadedTests(unittest.TestCase):

    @skip_if_broken_ubuntu_ssl
    def test_echo(self):
        """Basic test of an SSL client connecting to a server"""
        if support.verbose:
            sys.stdout.write("\n")
        for protocol in PROTOCOLS:
            if protocol in {ssl.PROTOCOL_TLS_CLIENT, ssl.PROTOCOL_TLS_SERVER}:
                continue
            with self.subTest(protocol=ssl._PROTOCOL_NAMES[protocol]):
                context = ssl.SSLContext(protocol)
                context.load_cert_chain(CERTFILE)
                server_params_test(context, context,
                                   chatty=True, connectionchatty=True)

        client_context, server_context, hostname = testing_context()

        with self.subTest(client=ssl.PROTOCOL_TLS_CLIENT, server=ssl.PROTOCOL_TLS_SERVER):
            server_params_test(client_context=client_context,
                               server_context=server_context,
                               chatty=True, connectionchatty=True,
                               sni_name=hostname)

        client_context.check_hostname = False
        with self.subTest(client=ssl.PROTOCOL_TLS_SERVER, server=ssl.PROTOCOL_TLS_CLIENT):
            with self.assertRaises(ssl.SSLError) as e:
                server_params_test(client_context=server_context,
                                   server_context=client_context,
                                   chatty=True, connectionchatty=True,
                                   sni_name=hostname)
            self.assertIn('called a function you should not call',
                          str(e.exception))

        with self.subTest(client=ssl.PROTOCOL_TLS_SERVER, server=ssl.PROTOCOL_TLS_SERVER):
            with self.assertRaises(ssl.SSLError) as e:
                server_params_test(client_context=server_context,
                                   server_context=server_context,
                                   chatty=True, connectionchatty=True)
            self.assertIn('called a function you should not call',
                          str(e.exception))

        with self.subTest(client=ssl.PROTOCOL_TLS_CLIENT, server=ssl.PROTOCOL_TLS_CLIENT):
            with self.assertRaises(ssl.SSLError) as e:
                server_params_test(client_context=server_context,
                                   server_context=client_context,
                                   chatty=True, connectionchatty=True)
            self.assertIn('called a function you should not call',
                          str(e.exception))

    def test_getpeercert(self):
        if support.verbose:
            sys.stdout.write("\n")

        client_context, server_context, hostname = testing_context()
        server = ThreadedEchoServer(context=server_context, chatty=False)
        with server:
            with client_context.wrap_socket(socket.socket(),
                                            do_handshake_on_connect=False,
                                            server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                # getpeercert() raise ValueError while the handshake isn't
                # done.
                with self.assertRaises(ValueError):
                    s.getpeercert()
                s.do_handshake()
                cert = s.getpeercert()
                self.assertTrue(cert, "Can't get peer certificate.")
                cipher = s.cipher()
                if support.verbose:
                    sys.stdout.write(pprint.pformat(cert) + '\n')
                    sys.stdout.write("Connection cipher is " + str(cipher) + '.\n')
                if 'subject' not in cert:
                    self.fail("No subject field in certificate: %s." %
                              pprint.pformat(cert))
                if ((('organizationName', 'Python Software Foundation'),)
                    not in cert['subject']):
                    self.fail(
                        "Missing or invalid 'organizationName' field in certificate subject; "
                        "should be 'Python Software Foundation'.")
                self.assertIn('notBefore', cert)
                self.assertIn('notAfter', cert)
                before = ssl.cert_time_to_seconds(cert['notBefore'])
                after = ssl.cert_time_to_seconds(cert['notAfter'])
                self.assertLess(before, after)

    @unittest.skipUnless(have_verify_flags(),
                        "verify_flags need OpenSSL > 0.9.8")
    def test_crl_check(self):
        if support.verbose:
            sys.stdout.write("\n")

        client_context, server_context, hostname = testing_context()

        tf = getattr(ssl, "VERIFY_X509_TRUSTED_FIRST", 0)
        self.assertEqual(client_context.verify_flags, ssl.VERIFY_DEFAULT | tf)

        # VERIFY_DEFAULT should pass
        server = ThreadedEchoServer(context=server_context, chatty=True)
        with server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                cert = s.getpeercert()
                self.assertTrue(cert, "Can't get peer certificate.")

        # VERIFY_CRL_CHECK_LEAF without a loaded CRL file fails
        client_context.verify_flags |= ssl.VERIFY_CRL_CHECK_LEAF

        server = ThreadedEchoServer(context=server_context, chatty=True)
        with server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                with self.assertRaisesRegex(ssl.SSLError,
                                            "certificate verify failed"):
                    s.connect((HOST, server.port))

        # now load a CRL file. The CRL file is signed by the CA.
        client_context.load_verify_locations(CRLFILE)

        server = ThreadedEchoServer(context=server_context, chatty=True)
        with server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                cert = s.getpeercert()
                self.assertTrue(cert, "Can't get peer certificate.")

    def test_check_hostname(self):
        if support.verbose:
            sys.stdout.write("\n")

        client_context, server_context, hostname = testing_context()

        # correct hostname should verify
        server = ThreadedEchoServer(context=server_context, chatty=True)
        with server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                cert = s.getpeercert()
                self.assertTrue(cert, "Can't get peer certificate.")

        # incorrect hostname should raise an exception
        server = ThreadedEchoServer(context=server_context, chatty=True)
        with server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname="invalid") as s:
                with self.assertRaisesRegex(
                        ssl.CertificateError,
                        "Hostname mismatch, certificate is not valid for 'invalid'."):
                    s.connect((HOST, server.port))

        # missing server_hostname arg should cause an exception, too
        server = ThreadedEchoServer(context=server_context, chatty=True)
        with server:
            with socket.socket() as s:
                with self.assertRaisesRegex(ValueError,
                                            "check_hostname requires server_hostname"):
                    client_context.wrap_socket(s)

    def test_ecc_cert(self):
        client_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        client_context.load_verify_locations(SIGNING_CA)
        client_context.set_ciphers(
            'TLS13-AES-128-GCM-SHA256:TLS13-CHACHA20-POLY1305-SHA256:'
            'ECDHE:ECDSA:!NULL:!aRSA'
        )
        hostname = SIGNED_CERTFILE_ECC_HOSTNAME

        server_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        # load ECC cert
        server_context.load_cert_chain(SIGNED_CERTFILE_ECC)

        # correct hostname should verify
        server = ThreadedEchoServer(context=server_context, chatty=True)
        with server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                cert = s.getpeercert()
                self.assertTrue(cert, "Can't get peer certificate.")
                cipher = s.cipher()[0].split('-')
                self.assertTrue(cipher[:2], ('ECDHE', 'ECDSA'))

    def test_dual_rsa_ecc(self):
        client_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        client_context.load_verify_locations(SIGNING_CA)
        # TODO: fix TLSv1.3 once SSLContext can restrict signature
        #       algorithms.
        client_context.options |= ssl.OP_NO_TLSv1_3
        # only ECDSA certs
        client_context.set_ciphers('ECDHE:ECDSA:!NULL:!aRSA')
        hostname = SIGNED_CERTFILE_ECC_HOSTNAME

        server_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        # load ECC and RSA key/cert pairs
        server_context.load_cert_chain(SIGNED_CERTFILE_ECC)
        server_context.load_cert_chain(SIGNED_CERTFILE)

        # correct hostname should verify
        server = ThreadedEchoServer(context=server_context, chatty=True)
        with server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                cert = s.getpeercert()
                self.assertTrue(cert, "Can't get peer certificate.")
                cipher = s.cipher()[0].split('-')
                self.assertTrue(cipher[:2], ('ECDHE', 'ECDSA'))

    def test_check_hostname_idn(self):
        if support.verbose:
            sys.stdout.write("\n")

        server_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        server_context.load_cert_chain(IDNSANSFILE)
        # TODO: fix TLSv1.3 support
        server_context.options |= ssl.OP_NO_TLSv1_3

        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        context.verify_mode = ssl.CERT_REQUIRED
        context.check_hostname = True
        context.load_verify_locations(SIGNING_CA)

        # correct hostname should verify, when specified in several
        # different ways
        idn_hostnames = [
            ('könig.idn.pythontest.net',
             'xn--knig-5qa.idn.pythontest.net'),
            ('xn--knig-5qa.idn.pythontest.net',
             'xn--knig-5qa.idn.pythontest.net'),
            (b'xn--knig-5qa.idn.pythontest.net',
             'xn--knig-5qa.idn.pythontest.net'),

            ('königsgäßchen.idna2003.pythontest.net',
             'xn--knigsgsschen-lcb0w.idna2003.pythontest.net'),
            ('xn--knigsgsschen-lcb0w.idna2003.pythontest.net',
             'xn--knigsgsschen-lcb0w.idna2003.pythontest.net'),
            (b'xn--knigsgsschen-lcb0w.idna2003.pythontest.net',
             'xn--knigsgsschen-lcb0w.idna2003.pythontest.net'),

            # ('königsgäßchen.idna2008.pythontest.net',
            #  'xn--knigsgchen-b4a3dun.idna2008.pythontest.net'),
            ('xn--knigsgchen-b4a3dun.idna2008.pythontest.net',
             'xn--knigsgchen-b4a3dun.idna2008.pythontest.net'),
            (b'xn--knigsgchen-b4a3dun.idna2008.pythontest.net',
             'xn--knigsgchen-b4a3dun.idna2008.pythontest.net'),

        ]
        for server_hostname, expected_hostname in idn_hostnames:
            server = ThreadedEchoServer(context=server_context, chatty=True)
            with server:
                with context.wrap_socket(socket.socket(),
                                         server_hostname=server_hostname) as s:
                    self.assertEqual(s.server_hostname, expected_hostname)
                    s.connect((HOST, server.port))
                    cert = s.getpeercert()
                    self.assertEqual(s.server_hostname, expected_hostname)
                    self.assertTrue(cert, "Can't get peer certificate.")

        # incorrect hostname should raise an exception
        server = ThreadedEchoServer(context=server_context, chatty=True)
        with server:
            with context.wrap_socket(socket.socket(),
                                     server_hostname="python.example.org") as s:
                with self.assertRaises(ssl.CertificateError):
                    s.connect((HOST, server.port))

    def test_wrong_cert(self):
        """Connecting when the server rejects the client's certificate

        Launch a server with CERT_REQUIRED, and check that trying to
        connect to it with a wrong client certificate fails.
        """
        client_context, server_context, hostname = testing_context()
        # load client cert
        client_context.load_cert_chain(WRONG_CERT)
        # require TLS client authentication
        server_context.verify_mode = ssl.CERT_REQUIRED
        # TODO: fix TLSv1.3 support
        # With TLS 1.3, test fails with exception in server thread
        server_context.options |= ssl.OP_NO_TLSv1_3

        server = ThreadedEchoServer(
            context=server_context, chatty=True, connectionchatty=True,
        )

        with server, \
                client_context.wrap_socket(socket.socket(),
                                           server_hostname=hostname) as s:
            try:
                # Expect either an SSL error about the server rejecting
                # the connection, or a low-level connection reset (which
                # sometimes happens on Windows)
                s.connect((HOST, server.port))
            except ssl.SSLError as e:
                if support.verbose:
                    sys.stdout.write("\nSSLError is %r\n" % e)
            except OSError as e:
                if e.errno != errno.ECONNRESET:
                    raise
                if support.verbose:
                    sys.stdout.write("\nsocket.error is %r\n" % e)
            else:
                self.fail("Use of invalid cert should have failed!")

    def test_rude_shutdown(self):
        """A brutal shutdown of an SSL server should raise an OSError
        in the client when attempting handshake.
        """
        listener_ready = threading.Event()
        listener_gone = threading.Event()

        s = socket.socket()
        port = support.bind_port(s, HOST)

        # `listener` runs in a thread.  It sits in an accept() until
        # the main thread connects.  Then it rudely closes the socket,
        # and sets Event `listener_gone` to let the main thread know
        # the socket is gone.
        def listener():
            s.listen()
            listener_ready.set()
            newsock, addr = s.accept()
            newsock.close()
            s.close()
            listener_gone.set()

        def connector():
            listener_ready.wait()
            with socket.socket() as c:
                c.connect((HOST, port))
                listener_gone.wait()
                try:
                    ssl_sock = test_wrap_socket(c)
                except OSError:
                    pass
                else:
                    self.fail('connecting to closed SSL socket should have failed')

        t = threading.Thread(target=listener)
        t.start()
        try:
            connector()
        finally:
            t.join()

    def test_ssl_cert_verify_error(self):
        if support.verbose:
            sys.stdout.write("\n")

        server_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        server_context.load_cert_chain(SIGNED_CERTFILE)

        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)

        server = ThreadedEchoServer(context=server_context, chatty=True)
        with server:
            with context.wrap_socket(socket.socket(),
                                     server_hostname=SIGNED_CERTFILE_HOSTNAME) as s:
                try:
                    s.connect((HOST, server.port))
                except ssl.SSLError as e:
                    msg = 'unable to get local issuer certificate'
                    self.assertIsInstance(e, ssl.SSLCertVerificationError)
                    self.assertEqual(e.verify_code, 20)
                    self.assertEqual(e.verify_message, msg)
                    self.assertIn(msg, repr(e))
                    self.assertIn('certificate verify failed', repr(e))

    @skip_if_broken_ubuntu_ssl
    @unittest.skipUnless(hasattr(ssl, 'PROTOCOL_SSLv2'),
                         "OpenSSL is compiled without SSLv2 support")
    def test_protocol_sslv2(self):
        """Connecting to an SSLv2 server with various client options"""
        if support.verbose:
            sys.stdout.write("\n")
        try_protocol_combo(ssl.PROTOCOL_SSLv2, ssl.PROTOCOL_SSLv2, True)
        try_protocol_combo(ssl.PROTOCOL_SSLv2, ssl.PROTOCOL_SSLv2, True, ssl.CERT_OPTIONAL)
        try_protocol_combo(ssl.PROTOCOL_SSLv2, ssl.PROTOCOL_SSLv2, True, ssl.CERT_REQUIRED)
        try_protocol_combo(ssl.PROTOCOL_SSLv2, ssl.PROTOCOL_TLS, False)
        if hasattr(ssl, 'PROTOCOL_SSLv3'):
            try_protocol_combo(ssl.PROTOCOL_SSLv2, ssl.PROTOCOL_SSLv3, False)
        try_protocol_combo(ssl.PROTOCOL_SSLv2, ssl.PROTOCOL_TLSv1, False)
        # SSLv23 client with specific SSL options
        if no_sslv2_implies_sslv3_hello():
            # No SSLv2 => client will use an SSLv3 hello on recent OpenSSLs
            try_protocol_combo(ssl.PROTOCOL_SSLv2, ssl.PROTOCOL_TLS, False,
                               client_options=ssl.OP_NO_SSLv2)
        try_protocol_combo(ssl.PROTOCOL_SSLv2, ssl.PROTOCOL_TLS, False,
                           client_options=ssl.OP_NO_SSLv3)
        try_protocol_combo(ssl.PROTOCOL_SSLv2, ssl.PROTOCOL_TLS, False,
                           client_options=ssl.OP_NO_TLSv1)

    @skip_if_broken_ubuntu_ssl
    def test_PROTOCOL_TLS(self):
        """Connecting to an SSLv23 server with various client options"""
        if support.verbose:
            sys.stdout.write("\n")
        if hasattr(ssl, 'PROTOCOL_SSLv2'):
            try:
                try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_SSLv2, True)
            except OSError as x:
                # this fails on some older versions of OpenSSL (0.9.7l, for instance)
                if support.verbose:
                    sys.stdout.write(
                        " SSL2 client to SSL23 server test unexpectedly failed:\n %s\n"
                        % str(x))
        if hasattr(ssl, 'PROTOCOL_SSLv3'):
            try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_SSLv3, False)
        try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_TLS, True)
        try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_TLSv1, 'TLSv1')

        if hasattr(ssl, 'PROTOCOL_SSLv3'):
            try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_SSLv3, False, ssl.CERT_OPTIONAL)
        try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_TLS, True, ssl.CERT_OPTIONAL)
        try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_TLSv1, 'TLSv1', ssl.CERT_OPTIONAL)

        if hasattr(ssl, 'PROTOCOL_SSLv3'):
            try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_SSLv3, False, ssl.CERT_REQUIRED)
        try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_TLS, True, ssl.CERT_REQUIRED)
        try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_TLSv1, 'TLSv1', ssl.CERT_REQUIRED)

        # Server with specific SSL options
        if hasattr(ssl, 'PROTOCOL_SSLv3'):
            try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_SSLv3, False,
                           server_options=ssl.OP_NO_SSLv3)
        # Will choose TLSv1
        try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_TLS, True,
                           server_options=ssl.OP_NO_SSLv2 | ssl.OP_NO_SSLv3)
        try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_TLSv1, False,
                           server_options=ssl.OP_NO_TLSv1)


    @skip_if_broken_ubuntu_ssl
    @unittest.skipUnless(hasattr(ssl, 'PROTOCOL_SSLv3'),
                         "OpenSSL is compiled without SSLv3 support")
    def test_protocol_sslv3(self):
        """Connecting to an SSLv3 server with various client options"""
        if support.verbose:
            sys.stdout.write("\n")
        try_protocol_combo(ssl.PROTOCOL_SSLv3, ssl.PROTOCOL_SSLv3, 'SSLv3')
        try_protocol_combo(ssl.PROTOCOL_SSLv3, ssl.PROTOCOL_SSLv3, 'SSLv3', ssl.CERT_OPTIONAL)
        try_protocol_combo(ssl.PROTOCOL_SSLv3, ssl.PROTOCOL_SSLv3, 'SSLv3', ssl.CERT_REQUIRED)
        if hasattr(ssl, 'PROTOCOL_SSLv2'):
            try_protocol_combo(ssl.PROTOCOL_SSLv3, ssl.PROTOCOL_SSLv2, False)
        try_protocol_combo(ssl.PROTOCOL_SSLv3, ssl.PROTOCOL_TLS, False,
                           client_options=ssl.OP_NO_SSLv3)
        try_protocol_combo(ssl.PROTOCOL_SSLv3, ssl.PROTOCOL_TLSv1, False)
        if no_sslv2_implies_sslv3_hello():
            # No SSLv2 => client will use an SSLv3 hello on recent OpenSSLs
            try_protocol_combo(ssl.PROTOCOL_SSLv3, ssl.PROTOCOL_TLS,
                               False, client_options=ssl.OP_NO_SSLv2)

    @skip_if_broken_ubuntu_ssl
    def test_protocol_tlsv1(self):
        """Connecting to a TLSv1 server with various client options"""
        if support.verbose:
            sys.stdout.write("\n")
        try_protocol_combo(ssl.PROTOCOL_TLSv1, ssl.PROTOCOL_TLSv1, 'TLSv1')
        try_protocol_combo(ssl.PROTOCOL_TLSv1, ssl.PROTOCOL_TLSv1, 'TLSv1', ssl.CERT_OPTIONAL)
        try_protocol_combo(ssl.PROTOCOL_TLSv1, ssl.PROTOCOL_TLSv1, 'TLSv1', ssl.CERT_REQUIRED)
        if hasattr(ssl, 'PROTOCOL_SSLv2'):
            try_protocol_combo(ssl.PROTOCOL_TLSv1, ssl.PROTOCOL_SSLv2, False)
        if hasattr(ssl, 'PROTOCOL_SSLv3'):
            try_protocol_combo(ssl.PROTOCOL_TLSv1, ssl.PROTOCOL_SSLv3, False)
        try_protocol_combo(ssl.PROTOCOL_TLSv1, ssl.PROTOCOL_TLS, False,
                           client_options=ssl.OP_NO_TLSv1)

    @skip_if_broken_ubuntu_ssl
    @unittest.skipUnless(hasattr(ssl, "PROTOCOL_TLSv1_1"),
                         "TLS version 1.1 not supported.")
    def test_protocol_tlsv1_1(self):
        """Connecting to a TLSv1.1 server with various client options.
           Testing against older TLS versions."""
        if support.verbose:
            sys.stdout.write("\n")
        try_protocol_combo(ssl.PROTOCOL_TLSv1_1, ssl.PROTOCOL_TLSv1_1, 'TLSv1.1')
        if hasattr(ssl, 'PROTOCOL_SSLv2'):
            try_protocol_combo(ssl.PROTOCOL_TLSv1_1, ssl.PROTOCOL_SSLv2, False)
        if hasattr(ssl, 'PROTOCOL_SSLv3'):
            try_protocol_combo(ssl.PROTOCOL_TLSv1_1, ssl.PROTOCOL_SSLv3, False)
        try_protocol_combo(ssl.PROTOCOL_TLSv1_1, ssl.PROTOCOL_TLS, False,
                           client_options=ssl.OP_NO_TLSv1_1)

        try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_TLSv1_1, 'TLSv1.1')
        try_protocol_combo(ssl.PROTOCOL_TLSv1_1, ssl.PROTOCOL_TLSv1, False)
        try_protocol_combo(ssl.PROTOCOL_TLSv1, ssl.PROTOCOL_TLSv1_1, False)

    @skip_if_broken_ubuntu_ssl
    @unittest.skipUnless(hasattr(ssl, "PROTOCOL_TLSv1_2"),
                         "TLS version 1.2 not supported.")
    def test_protocol_tlsv1_2(self):
        """Connecting to a TLSv1.2 server with various client options.
           Testing against older TLS versions."""
        if support.verbose:
            sys.stdout.write("\n")
        try_protocol_combo(ssl.PROTOCOL_TLSv1_2, ssl.PROTOCOL_TLSv1_2, 'TLSv1.2',
                           server_options=ssl.OP_NO_SSLv3|ssl.OP_NO_SSLv2,
                           client_options=ssl.OP_NO_SSLv3|ssl.OP_NO_SSLv2,)
        if hasattr(ssl, 'PROTOCOL_SSLv2'):
            try_protocol_combo(ssl.PROTOCOL_TLSv1_2, ssl.PROTOCOL_SSLv2, False)
        if hasattr(ssl, 'PROTOCOL_SSLv3'):
            try_protocol_combo(ssl.PROTOCOL_TLSv1_2, ssl.PROTOCOL_SSLv3, False)
        try_protocol_combo(ssl.PROTOCOL_TLSv1_2, ssl.PROTOCOL_TLS, False,
                           client_options=ssl.OP_NO_TLSv1_2)

        try_protocol_combo(ssl.PROTOCOL_TLS, ssl.PROTOCOL_TLSv1_2, 'TLSv1.2')
        try_protocol_combo(ssl.PROTOCOL_TLSv1_2, ssl.PROTOCOL_TLSv1, False)
        try_protocol_combo(ssl.PROTOCOL_TLSv1, ssl.PROTOCOL_TLSv1_2, False)
        try_protocol_combo(ssl.PROTOCOL_TLSv1_2, ssl.PROTOCOL_TLSv1_1, False)
        try_protocol_combo(ssl.PROTOCOL_TLSv1_1, ssl.PROTOCOL_TLSv1_2, False)

    def test_starttls(self):
        """Switching from clear text to encrypted and back again."""
        msgs = (b"msg 1", b"MSG 2", b"STARTTLS", b"MSG 3", b"msg 4", b"ENDTLS", b"msg 5", b"msg 6")

        server = ThreadedEchoServer(CERTFILE,
                                    starttls_server=True,
                                    chatty=True,
                                    connectionchatty=True)
        wrapped = False
        with server:
            s = socket.socket()
            s.setblocking(1)
            s.connect((HOST, server.port))
            if support.verbose:
                sys.stdout.write("\n")
            for indata in msgs:
                if support.verbose:
                    sys.stdout.write(
                        " client:  sending %r...\n" % indata)
                if wrapped:
                    conn.write(indata)
                    outdata = conn.read()
                else:
                    s.send(indata)
                    outdata = s.recv(1024)
                msg = outdata.strip().lower()
                if indata == b"STARTTLS" and msg.startswith(b"ok"):
                    # STARTTLS ok, switch to secure mode
                    if support.verbose:
                        sys.stdout.write(
                            " client:  read %r from server, starting TLS...\n"
                            % msg)
                    conn = test_wrap_socket(s)
                    wrapped = True
                elif indata == b"ENDTLS" and msg.startswith(b"ok"):
                    # ENDTLS ok, switch back to clear text
                    if support.verbose:
                        sys.stdout.write(
                            " client:  read %r from server, ending TLS...\n"
                            % msg)
                    s = conn.unwrap()
                    wrapped = False
                else:
                    if support.verbose:
                        sys.stdout.write(
                            " client:  read %r from server\n" % msg)
            if support.verbose:
                sys.stdout.write(" client:  closing connection.\n")
            if wrapped:
                conn.write(b"over\n")
            else:
                s.send(b"over\n")
            if wrapped:
                conn.close()
            else:
                s.close()

    def test_socketserver(self):
        """Using socketserver to create and manage SSL connections."""
        server = make_https_server(self, certfile=SIGNED_CERTFILE)
        # try to connect
        if support.verbose:
            sys.stdout.write('\n')
        with open(CERTFILE, 'rb') as f:
            d1 = f.read()
        d2 = ''
        # now fetch the same data from the HTTPS server
        url = 'https://localhost:%d/%s' % (
            server.port, os.path.split(CERTFILE)[1])
        context = ssl.create_default_context(cafile=SIGNING_CA)
        f = urllib.request.urlopen(url, context=context)
        try:
            dlen = f.info().get("content-length")
            if dlen and (int(dlen) > 0):
                d2 = f.read(int(dlen))
                if support.verbose:
                    sys.stdout.write(
                        " client: read %d bytes from remote server '%s'\n"
                        % (len(d2), server))
        finally:
            f.close()
        self.assertEqual(d1, d2)

    def test_asyncore_server(self):
        """Check the example asyncore integration."""
        if support.verbose:
            sys.stdout.write("\n")

        indata = b"FOO\n"
        server = AsyncoreEchoServer(CERTFILE)
        with server:
            s = test_wrap_socket(socket.socket())
            s.connect(('127.0.0.1', server.port))
            if support.verbose:
                sys.stdout.write(
                    " client:  sending %r...\n" % indata)
            s.write(indata)
            outdata = s.read()
            if support.verbose:
                sys.stdout.write(" client:  read %r\n" % outdata)
            if outdata != indata.lower():
                self.fail(
                    "bad data <<%r>> (%d) received; expected <<%r>> (%d)\n"
                    % (outdata[:20], len(outdata),
                       indata[:20].lower(), len(indata)))
            s.write(b"over\n")
            if support.verbose:
                sys.stdout.write(" client:  closing connection.\n")
            s.close()
            if support.verbose:
                sys.stdout.write(" client:  connection closed.\n")

    def test_recv_send(self):
        """Test recv(), send() and friends."""
        if support.verbose:
            sys.stdout.write("\n")

        server = ThreadedEchoServer(CERTFILE,
                                    certreqs=ssl.CERT_NONE,
                                    ssl_version=ssl.PROTOCOL_TLS_SERVER,
                                    cacerts=CERTFILE,
                                    chatty=True,
                                    connectionchatty=False)
        with server:
            s = test_wrap_socket(socket.socket(),
                                server_side=False,
                                certfile=CERTFILE,
                                ca_certs=CERTFILE,
                                cert_reqs=ssl.CERT_NONE,
                                ssl_version=ssl.PROTOCOL_TLS_CLIENT)
            s.connect((HOST, server.port))
            # helper methods for standardising recv* method signatures
            def _recv_into():
                b = bytearray(b"\0"*100)
                count = s.recv_into(b)
                return b[:count]

            def _recvfrom_into():
                b = bytearray(b"\0"*100)
                count, addr = s.recvfrom_into(b)
                return b[:count]

            # (name, method, expect success?, *args, return value func)
            send_methods = [
                ('send', s.send, True, [], len),
                ('sendto', s.sendto, False, ["some.address"], len),
                ('sendall', s.sendall, True, [], lambda x: None),
            ]
            # (name, method, whether to expect success, *args)
            recv_methods = [
                ('recv', s.recv, True, []),
                ('recvfrom', s.recvfrom, False, ["some.address"]),
                ('recv_into', _recv_into, True, []),
                ('recvfrom_into', _recvfrom_into, False, []),
            ]
            data_prefix = "PREFIX_"

            for (meth_name, send_meth, expect_success, args,
                    ret_val_meth) in send_methods:
                indata = (data_prefix + meth_name).encode('ascii')
                try:
                    ret = send_meth(indata, *args)
                    msg = "sending with {}".format(meth_name)
                    self.assertEqual(ret, ret_val_meth(indata), msg=msg)
                    outdata = s.read()
                    if outdata != indata.lower():
                        self.fail(
                            "While sending with <<{name:s}>> bad data "
                            "<<{outdata:r}>> ({nout:d}) received; "
                            "expected <<{indata:r}>> ({nin:d})\n".format(
                                name=meth_name, outdata=outdata[:20],
                                nout=len(outdata),
                                indata=indata[:20], nin=len(indata)
                            )
                        )
                except ValueError as e:
                    if expect_success:
                        self.fail(
                            "Failed to send with method <<{name:s}>>; "
                            "expected to succeed.\n".format(name=meth_name)
                        )
                    if not str(e).startswith(meth_name):
                        self.fail(
                            "Method <<{name:s}>> failed with unexpected "
                            "exception message: {exp:s}\n".format(
                                name=meth_name, exp=e
                            )
                        )

            for meth_name, recv_meth, expect_success, args in recv_methods:
                indata = (data_prefix + meth_name).encode('ascii')
                try:
                    s.send(indata)
                    outdata = recv_meth(*args)
                    if outdata != indata.lower():
                        self.fail(
                            "While receiving with <<{name:s}>> bad data "
                            "<<{outdata:r}>> ({nout:d}) received; "
                            "expected <<{indata:r}>> ({nin:d})\n".format(
                                name=meth_name, outdata=outdata[:20],
                                nout=len(outdata),
                                indata=indata[:20], nin=len(indata)
                            )
                        )
                except ValueError as e:
                    if expect_success:
                        self.fail(
                            "Failed to receive with method <<{name:s}>>; "
                            "expected to succeed.\n".format(name=meth_name)
                        )
                    if not str(e).startswith(meth_name):
                        self.fail(
                            "Method <<{name:s}>> failed with unexpected "
                            "exception message: {exp:s}\n".format(
                                name=meth_name, exp=e
                            )
                        )
                    # consume data
                    s.read()

            # read(-1, buffer) is supported, even though read(-1) is not
            data = b"data"
            s.send(data)
            buffer = bytearray(len(data))
            self.assertEqual(s.read(-1, buffer), len(data))
            self.assertEqual(buffer, data)

            # sendall accepts bytes-like objects
            if ctypes is not None:
                ubyte = ctypes.c_ubyte * len(data)
                byteslike = ubyte.from_buffer_copy(data)
                s.sendall(byteslike)
                self.assertEqual(s.read(), data)

            # Make sure sendmsg et al are disallowed to avoid
            # inadvertent disclosure of data and/or corruption
            # of the encrypted data stream
            self.assertRaises(NotImplementedError, s.sendmsg, [b"data"])
            self.assertRaises(NotImplementedError, s.recvmsg, 100)
            self.assertRaises(NotImplementedError,
                              s.recvmsg_into, bytearray(100))
            s.write(b"over\n")

            self.assertRaises(ValueError, s.recv, -1)
            self.assertRaises(ValueError, s.read, -1)

            s.close()

    def test_recv_zero(self):
        server = ThreadedEchoServer(CERTFILE)
        server.__enter__()
        self.addCleanup(server.__exit__, None, None)
        s = socket.create_connection((HOST, server.port))
        self.addCleanup(s.close)
        s = test_wrap_socket(s, suppress_ragged_eofs=False)
        self.addCleanup(s.close)

        # recv/read(0) should return no data
        s.send(b"data")
        self.assertEqual(s.recv(0), b"")
        self.assertEqual(s.read(0), b"")
        self.assertEqual(s.read(), b"data")

        # Should not block if the other end sends no data
        s.setblocking(False)
        self.assertEqual(s.recv(0), b"")
        self.assertEqual(s.recv_into(bytearray()), 0)

    def test_nonblocking_send(self):
        server = ThreadedEchoServer(CERTFILE,
                                    certreqs=ssl.CERT_NONE,
                                    ssl_version=ssl.PROTOCOL_TLS_SERVER,
                                    cacerts=CERTFILE,
                                    chatty=True,
                                    connectionchatty=False)
        with server:
            s = test_wrap_socket(socket.socket(),
                                server_side=False,
                                certfile=CERTFILE,
                                ca_certs=CERTFILE,
                                cert_reqs=ssl.CERT_NONE,
                                ssl_version=ssl.PROTOCOL_TLS_CLIENT)
            s.connect((HOST, server.port))
            s.setblocking(False)

            # If we keep sending data, at some point the buffers
            # will be full and the call will block
            buf = bytearray(8192)
            def fill_buffer():
                while True:
                    s.send(buf)
            self.assertRaises((ssl.SSLWantWriteError,
                               ssl.SSLWantReadError), fill_buffer)

            # Now read all the output and discard it
            s.setblocking(True)
            s.close()

    def test_handshake_timeout(self):
        # Issue #5103: SSL handshake must respect the socket timeout
        server = socket.socket(socket.AF_INET)
        host = "127.0.0.1"
        port = support.bind_port(server)
        started = threading.Event()
        finish = False

        def serve():
            server.listen()
            started.set()
            conns = []
            while not finish:
                r, w, e = select.select([server], [], [], 0.1)
                if server in r:
                    # Let the socket hang around rather than having
                    # it closed by garbage collection.
                    conns.append(server.accept()[0])
            for sock in conns:
                sock.close()

        t = threading.Thread(target=serve)
        t.start()
        started.wait()

        try:
            try:
                c = socket.socket(socket.AF_INET)
                c.settimeout(0.2)
                c.connect((host, port))
                # Will attempt handshake and time out
                self.assertRaisesRegex(socket.timeout, "timed out",
                                       test_wrap_socket, c)
            finally:
                c.close()
            try:
                c = socket.socket(socket.AF_INET)
                c = test_wrap_socket(c)
                c.settimeout(0.2)
                # Will attempt handshake and time out
                self.assertRaisesRegex(socket.timeout, "timed out",
                                       c.connect, (host, port))
            finally:
                c.close()
        finally:
            finish = True
            t.join()
            server.close()

    def test_server_accept(self):
        # Issue #16357: accept() on a SSLSocket created through
        # SSLContext.wrap_socket().
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        context.verify_mode = ssl.CERT_REQUIRED
        context.load_verify_locations(SIGNING_CA)
        context.load_cert_chain(SIGNED_CERTFILE)
        server = socket.socket(socket.AF_INET)
        host = "127.0.0.1"
        port = support.bind_port(server)
        server = context.wrap_socket(server, server_side=True)
        self.assertTrue(server.server_side)

        evt = threading.Event()
        remote = None
        peer = None
        def serve():
            nonlocal remote, peer
            server.listen()
            # Block on the accept and wait on the connection to close.
            evt.set()
            remote, peer = server.accept()
            remote.recv(1)

        t = threading.Thread(target=serve)
        t.start()
        # Client wait until server setup and perform a connect.
        evt.wait()
        client = context.wrap_socket(socket.socket())
        client.connect((host, port))
        client_addr = client.getsockname()
        client.close()
        t.join()
        remote.close()
        server.close()
        # Sanity checks.
        self.assertIsInstance(remote, ssl.SSLSocket)
        self.assertEqual(peer, client_addr)

    def test_getpeercert_enotconn(self):
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        with context.wrap_socket(socket.socket()) as sock:
            with self.assertRaises(OSError) as cm:
                sock.getpeercert()
            self.assertEqual(cm.exception.errno, errno.ENOTCONN)

    def test_do_handshake_enotconn(self):
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        with context.wrap_socket(socket.socket()) as sock:
            with self.assertRaises(OSError) as cm:
                sock.do_handshake()
            self.assertEqual(cm.exception.errno, errno.ENOTCONN)

    def test_default_ciphers(self):
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        try:
            # Force a set of weak ciphers on our client context
            context.set_ciphers("DES")
        except ssl.SSLError:
            self.skipTest("no DES cipher available")
        with ThreadedEchoServer(CERTFILE,
                                ssl_version=ssl.PROTOCOL_TLS,
                                chatty=False) as server:
            with context.wrap_socket(socket.socket()) as s:
                with self.assertRaises(OSError):
                    s.connect((HOST, server.port))
        self.assertIn("no shared cipher", server.conn_errors[0])

    def test_version_basic(self):
        """
        Basic tests for SSLSocket.version().
        More tests are done in the test_protocol_*() methods.
        """
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        context.check_hostname = False
        context.verify_mode = ssl.CERT_NONE
        with ThreadedEchoServer(CERTFILE,
                                ssl_version=ssl.PROTOCOL_TLS_SERVER,
                                chatty=False) as server:
            with context.wrap_socket(socket.socket()) as s:
                self.assertIs(s.version(), None)
                self.assertIs(s._sslobj, None)
                s.connect((HOST, server.port))
                if ssl.OPENSSL_VERSION_INFO >= (1, 1, 1):
                    self.assertEqual(s.version(), 'TLSv1.3')
                elif ssl.OPENSSL_VERSION_INFO >= (1, 0, 2):
                    self.assertEqual(s.version(), 'TLSv1.2')
                else:  # 0.9.8 to 1.0.1
                    self.assertIn(s.version(), ('TLSv1', 'TLSv1.2'))
            self.assertIs(s._sslobj, None)
            self.assertIs(s.version(), None)

    @unittest.skipUnless(ssl.HAS_TLSv1_3,
                         "test requires TLSv1.3 enabled OpenSSL")
    def test_tls1_3(self):
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        context.load_cert_chain(CERTFILE)
        context.options |= (
            ssl.OP_NO_TLSv1 | ssl.OP_NO_TLSv1_1 | ssl.OP_NO_TLSv1_2
        )
        with ThreadedEchoServer(context=context) as server:
            with context.wrap_socket(socket.socket()) as s:
                s.connect((HOST, server.port))
                self.assertIn(s.cipher()[0], {
                    'TLS13-AES-256-GCM-SHA384',
                    'TLS13-CHACHA20-POLY1305-SHA256',
                    'TLS13-AES-128-GCM-SHA256',
                })
                self.assertEqual(s.version(), 'TLSv1.3')

    @unittest.skipUnless(hasattr(ssl.SSLContext, 'minimum_version'),
                         "required OpenSSL 1.1.0g")
    def test_min_max_version(self):
        client_context, server_context, hostname = testing_context()
        # client TLSv1.0 to 1.2
        client_context.minimum_version = ssl.TLSVersion.TLSv1
        client_context.maximum_version = ssl.TLSVersion.TLSv1_2
        # server only TLSv1.2
        server_context.minimum_version = ssl.TLSVersion.TLSv1_2
        server_context.maximum_version = ssl.TLSVersion.TLSv1_2

        with ThreadedEchoServer(context=server_context) as server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                self.assertEqual(s.version(), 'TLSv1.2')

        # client 1.0 to 1.2, server 1.0 to 1.1
        server_context.minimum_version = ssl.TLSVersion.TLSv1
        server_context.maximum_version = ssl.TLSVersion.TLSv1_1

        with ThreadedEchoServer(context=server_context) as server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                self.assertEqual(s.version(), 'TLSv1.1')

        # client 1.0, server 1.2 (mismatch)
        server_context.minimum_version = ssl.TLSVersion.TLSv1_2
        server_context.maximum_version = ssl.TLSVersion.TLSv1_2
        client_context.minimum_version = ssl.TLSVersion.TLSv1
        client_context.maximum_version = ssl.TLSVersion.TLSv1
        with ThreadedEchoServer(context=server_context) as server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                with self.assertRaises(ssl.SSLError) as e:
                    s.connect((HOST, server.port))
                self.assertIn("alert", str(e.exception))


    @unittest.skipUnless(hasattr(ssl.SSLContext, 'minimum_version'),
                         "required OpenSSL 1.1.0g")
    @unittest.skipUnless(ssl.HAS_SSLv3, "requires SSLv3 support")
    def test_min_max_version_sslv3(self):
        client_context, server_context, hostname = testing_context()
        server_context.minimum_version = ssl.TLSVersion.SSLv3
        client_context.minimum_version = ssl.TLSVersion.SSLv3
        client_context.maximum_version = ssl.TLSVersion.SSLv3
        with ThreadedEchoServer(context=server_context) as server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                self.assertEqual(s.version(), 'SSLv3')

    @unittest.skipUnless(ssl.HAS_ECDH, "test requires ECDH-enabled OpenSSL")
    def test_default_ecdh_curve(self):
        # Issue #21015: elliptic curve-based Diffie Hellman key exchange
        # should be enabled by default on SSL contexts.
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        context.load_cert_chain(CERTFILE)
        # TLSv1.3 defaults to PFS key agreement and no longer has KEA in
        # cipher name.
        context.options |= ssl.OP_NO_TLSv1_3
        # Prior to OpenSSL 1.0.0, ECDH ciphers have to be enabled
        # explicitly using the 'ECCdraft' cipher alias.  Otherwise,
        # our default cipher list should prefer ECDH-based ciphers
        # automatically.
        if ssl.OPENSSL_VERSION_INFO < (1, 0, 0):
            context.set_ciphers("ECCdraft:ECDH")
        with ThreadedEchoServer(context=context) as server:
            with context.wrap_socket(socket.socket()) as s:
                s.connect((HOST, server.port))
                self.assertIn("ECDH", s.cipher()[0])

    @unittest.skipUnless("tls-unique" in ssl.CHANNEL_BINDING_TYPES,
                         "'tls-unique' channel binding not available")
    def test_tls_unique_channel_binding(self):
        """Test tls-unique channel binding."""
        if support.verbose:
            sys.stdout.write("\n")

        client_context, server_context, hostname = testing_context()
        # TODO: fix TLSv1.3 support
        client_context.options |= ssl.OP_NO_TLSv1_3

        server = ThreadedEchoServer(context=server_context,
                                    chatty=True,
                                    connectionchatty=False)

        with server:
            with client_context.wrap_socket(
                    socket.socket(),
                    server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                # get the data
                cb_data = s.get_channel_binding("tls-unique")
                if support.verbose:
                    sys.stdout.write(
                        " got channel binding data: {0!r}\n".format(cb_data))

                # check if it is sane
                self.assertIsNotNone(cb_data)
                self.assertEqual(len(cb_data), 12) # True for TLSv1

                # and compare with the peers version
                s.write(b"CB tls-unique\n")
                peer_data_repr = s.read().strip()
                self.assertEqual(peer_data_repr,
                                 repr(cb_data).encode("us-ascii"))

            # now, again
            with client_context.wrap_socket(
                    socket.socket(),
                    server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                new_cb_data = s.get_channel_binding("tls-unique")
                if support.verbose:
                    sys.stdout.write(
                        "got another channel binding data: {0!r}\n".format(
                            new_cb_data)
                    )
                # is it really unique
                self.assertNotEqual(cb_data, new_cb_data)
                self.assertIsNotNone(cb_data)
                self.assertEqual(len(cb_data), 12) # True for TLSv1
                s.write(b"CB tls-unique\n")
                peer_data_repr = s.read().strip()
                self.assertEqual(peer_data_repr,
                                 repr(new_cb_data).encode("us-ascii"))

    def test_compression(self):
        client_context, server_context, hostname = testing_context()
        stats = server_params_test(client_context, server_context,
                                   chatty=True, connectionchatty=True,
                                   sni_name=hostname)
        if support.verbose:
            sys.stdout.write(" got compression: {!r}\n".format(stats['compression']))
        self.assertIn(stats['compression'], { None, 'ZLIB', 'RLE' })

    @unittest.skipUnless(hasattr(ssl, 'OP_NO_COMPRESSION'),
                         "ssl.OP_NO_COMPRESSION needed for this test")
    def test_compression_disabled(self):
        client_context, server_context, hostname = testing_context()
        client_context.options |= ssl.OP_NO_COMPRESSION
        server_context.options |= ssl.OP_NO_COMPRESSION
        stats = server_params_test(client_context, server_context,
                                   chatty=True, connectionchatty=True,
                                   sni_name=hostname)
        self.assertIs(stats['compression'], None)

    def test_dh_params(self):
        # Check we can get a connection with ephemeral Diffie-Hellman
        client_context, server_context, hostname = testing_context()
        # test scenario needs TLS <= 1.2
        client_context.options |= ssl.OP_NO_TLSv1_3
        server_context.load_dh_params(DHFILE)
        server_context.set_ciphers("kEDH")
        server_context.options |= ssl.OP_NO_TLSv1_3
        stats = server_params_test(client_context, server_context,
                                   chatty=True, connectionchatty=True,
                                   sni_name=hostname)
        cipher = stats["cipher"][0]
        parts = cipher.split("-")
        if "ADH" not in parts and "EDH" not in parts and "DHE" not in parts:
            self.fail("Non-DH cipher: " + cipher[0])

    @unittest.skipUnless(HAVE_SECP_CURVES, "needs secp384r1 curve support")
    @unittest.skipIf(IS_OPENSSL_1_1_1, "TODO: Test doesn't work on 1.1.1")
    def test_ecdh_curve(self):
        # server secp384r1, client auto
        client_context, server_context, hostname = testing_context()

        server_context.set_ecdh_curve("secp384r1")
        server_context.set_ciphers("ECDHE:!eNULL:!aNULL")
        server_context.options |= ssl.OP_NO_TLSv1 | ssl.OP_NO_TLSv1_1
        stats = server_params_test(client_context, server_context,
                                   chatty=True, connectionchatty=True,
                                   sni_name=hostname)

        # server auto, client secp384r1
        client_context, server_context, hostname = testing_context()
        client_context.set_ecdh_curve("secp384r1")
        server_context.set_ciphers("ECDHE:!eNULL:!aNULL")
        server_context.options |= ssl.OP_NO_TLSv1 | ssl.OP_NO_TLSv1_1
        stats = server_params_test(client_context, server_context,
                                   chatty=True, connectionchatty=True,
                                   sni_name=hostname)

        # server / client curve mismatch
        client_context, server_context, hostname = testing_context()
        client_context.set_ecdh_curve("prime256v1")
        server_context.set_ecdh_curve("secp384r1")
        server_context.set_ciphers("ECDHE:!eNULL:!aNULL")
        server_context.options |= ssl.OP_NO_TLSv1 | ssl.OP_NO_TLSv1_1
        try:
            stats = server_params_test(client_context, server_context,
                                       chatty=True, connectionchatty=True,
                                       sni_name=hostname)
        except ssl.SSLError:
            pass
        else:
            # OpenSSL 1.0.2 does not fail although it should.
            if IS_OPENSSL_1_1_0:
                self.fail("mismatch curve did not fail")

    def test_selected_alpn_protocol(self):
        # selected_alpn_protocol() is None unless ALPN is used.
        client_context, server_context, hostname = testing_context()
        stats = server_params_test(client_context, server_context,
                                   chatty=True, connectionchatty=True,
                                   sni_name=hostname)
        self.assertIs(stats['client_alpn_protocol'], None)

    @unittest.skipUnless(ssl.HAS_ALPN, "ALPN support required")
    def test_selected_alpn_protocol_if_server_uses_alpn(self):
        # selected_alpn_protocol() is None unless ALPN is used by the client.
        client_context, server_context, hostname = testing_context()
        server_context.set_alpn_protocols(['foo', 'bar'])
        stats = server_params_test(client_context, server_context,
                                   chatty=True, connectionchatty=True,
                                   sni_name=hostname)
        self.assertIs(stats['client_alpn_protocol'], None)

    @unittest.skipUnless(ssl.HAS_ALPN, "ALPN support needed for this test")
    def test_alpn_protocols(self):
        server_protocols = ['foo', 'bar', 'milkshake']
        protocol_tests = [
            (['foo', 'bar'], 'foo'),
            (['bar', 'foo'], 'foo'),
            (['milkshake'], 'milkshake'),
            (['http/3.0', 'http/4.0'], None)
        ]
        for client_protocols, expected in protocol_tests:
            client_context, server_context, hostname = testing_context()
            server_context.set_alpn_protocols(server_protocols)
            client_context.set_alpn_protocols(client_protocols)

            try:
                stats = server_params_test(client_context,
                                           server_context,
                                           chatty=True,
                                           connectionchatty=True,
                                           sni_name=hostname)
            except ssl.SSLError as e:
                stats = e

            if (expected is None and IS_OPENSSL_1_1_0
                    and ssl.OPENSSL_VERSION_INFO < (1, 1, 0, 6)):
                # OpenSSL 1.1.0 to 1.1.0e raises handshake error
                self.assertIsInstance(stats, ssl.SSLError)
            else:
                msg = "failed trying %s (s) and %s (c).\n" \
                    "was expecting %s, but got %%s from the %%s" \
                        % (str(server_protocols), str(client_protocols),
                            str(expected))
                client_result = stats['client_alpn_protocol']
                self.assertEqual(client_result, expected,
                                 msg % (client_result, "client"))
                server_result = stats['server_alpn_protocols'][-1] \
                    if len(stats['server_alpn_protocols']) else 'nothing'
                self.assertEqual(server_result, expected,
                                 msg % (server_result, "server"))

    def test_selected_npn_protocol(self):
        # selected_npn_protocol() is None unless NPN is used
        client_context, server_context, hostname = testing_context()
        stats = server_params_test(client_context, server_context,
                                   chatty=True, connectionchatty=True,
                                   sni_name=hostname)
        self.assertIs(stats['client_npn_protocol'], None)

    @unittest.skipUnless(ssl.HAS_NPN, "NPN support needed for this test")
    def test_npn_protocols(self):
        server_protocols = ['http/1.1', 'spdy/2']
        protocol_tests = [
            (['http/1.1', 'spdy/2'], 'http/1.1'),
            (['spdy/2', 'http/1.1'], 'http/1.1'),
            (['spdy/2', 'test'], 'spdy/2'),
            (['abc', 'def'], 'abc')
        ]
        for client_protocols, expected in protocol_tests:
            client_context, server_context, hostname = testing_context()
            server_context.set_npn_protocols(server_protocols)
            client_context.set_npn_protocols(client_protocols)
            stats = server_params_test(client_context, server_context,
                                       chatty=True, connectionchatty=True,
                                       sni_name=hostname)
            msg = "failed trying %s (s) and %s (c).\n" \
                  "was expecting %s, but got %%s from the %%s" \
                      % (str(server_protocols), str(client_protocols),
                         str(expected))
            client_result = stats['client_npn_protocol']
            self.assertEqual(client_result, expected, msg % (client_result, "client"))
            server_result = stats['server_npn_protocols'][-1] \
                if len(stats['server_npn_protocols']) else 'nothing'
            self.assertEqual(server_result, expected, msg % (server_result, "server"))

    def sni_contexts(self):
        server_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        server_context.load_cert_chain(SIGNED_CERTFILE)
        other_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        other_context.load_cert_chain(SIGNED_CERTFILE2)
        client_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        client_context.load_verify_locations(SIGNING_CA)
        return server_context, other_context, client_context

    def check_common_name(self, stats, name):
        cert = stats['peercert']
        self.assertIn((('commonName', name),), cert['subject'])

    @needs_sni
    def test_sni_callback(self):
        calls = []
        server_context, other_context, client_context = self.sni_contexts()

        client_context.check_hostname = False

        def servername_cb(ssl_sock, server_name, initial_context):
            calls.append((server_name, initial_context))
            if server_name is not None:
                ssl_sock.context = other_context
        server_context.set_servername_callback(servername_cb)

        stats = server_params_test(client_context, server_context,
                                   chatty=True,
                                   sni_name='supermessage')
        # The hostname was fetched properly, and the certificate was
        # changed for the connection.
        self.assertEqual(calls, [("supermessage", server_context)])
        # CERTFILE4 was selected
        self.check_common_name(stats, 'fakehostname')

        calls = []
        # The callback is called with server_name=None
        stats = server_params_test(client_context, server_context,
                                   chatty=True,
                                   sni_name=None)
        self.assertEqual(calls, [(None, server_context)])
        self.check_common_name(stats, SIGNED_CERTFILE_HOSTNAME)

        # Check disabling the callback
        calls = []
        server_context.set_servername_callback(None)

        stats = server_params_test(client_context, server_context,
                                   chatty=True,
                                   sni_name='notfunny')
        # Certificate didn't change
        self.check_common_name(stats, SIGNED_CERTFILE_HOSTNAME)
        self.assertEqual(calls, [])

    @needs_sni
    def test_sni_callback_alert(self):
        # Returning a TLS alert is reflected to the connecting client
        server_context, other_context, client_context = self.sni_contexts()

        def cb_returning_alert(ssl_sock, server_name, initial_context):
            return ssl.ALERT_DESCRIPTION_ACCESS_DENIED
        server_context.set_servername_callback(cb_returning_alert)
        with self.assertRaises(ssl.SSLError) as cm:
            stats = server_params_test(client_context, server_context,
                                       chatty=False,
                                       sni_name='supermessage')
        self.assertEqual(cm.exception.reason, 'TLSV1_ALERT_ACCESS_DENIED')

    @needs_sni
    def test_sni_callback_raising(self):
        # Raising fails the connection with a TLS handshake failure alert.
        server_context, other_context, client_context = self.sni_contexts()

        def cb_raising(ssl_sock, server_name, initial_context):
            1/0
        server_context.set_servername_callback(cb_raising)

        with self.assertRaises(ssl.SSLError) as cm, \
             support.captured_stderr() as stderr:
            stats = server_params_test(client_context, server_context,
                                       chatty=False,
                                       sni_name='supermessage')
        self.assertEqual(cm.exception.reason, 'SSLV3_ALERT_HANDSHAKE_FAILURE')
        self.assertIn("ZeroDivisionError", stderr.getvalue())

    @needs_sni
    def test_sni_callback_wrong_return_type(self):
        # Returning the wrong return type terminates the TLS connection
        # with an internal error alert.
        server_context, other_context, client_context = self.sni_contexts()

        def cb_wrong_return_type(ssl_sock, server_name, initial_context):
            return "foo"
        server_context.set_servername_callback(cb_wrong_return_type)

        with self.assertRaises(ssl.SSLError) as cm, \
             support.captured_stderr() as stderr:
            stats = server_params_test(client_context, server_context,
                                       chatty=False,
                                       sni_name='supermessage')
        self.assertEqual(cm.exception.reason, 'TLSV1_ALERT_INTERNAL_ERROR')
        self.assertIn("TypeError", stderr.getvalue())

    def test_shared_ciphers(self):
        client_context, server_context, hostname = testing_context()
        if ssl.OPENSSL_VERSION_INFO >= (1, 0, 2):
            client_context.set_ciphers("AES128:AES256")
            server_context.set_ciphers("AES256")
            alg1 = "AES256"
            alg2 = "AES-256"
        else:
            client_context.set_ciphers("AES:3DES")
            server_context.set_ciphers("3DES")
            alg1 = "3DES"
            alg2 = "DES-CBC3"

        stats = server_params_test(client_context, server_context,
                                   sni_name=hostname)
        ciphers = stats['server_shared_ciphers'][0]
        self.assertGreater(len(ciphers), 0)
        for name, tls_version, bits in ciphers:
            if not alg1 in name.split("-") and alg2 not in name:
                self.fail(name)

    def test_read_write_after_close_raises_valuerror(self):
        client_context, server_context, hostname = testing_context()
        server = ThreadedEchoServer(context=server_context, chatty=False)

        with server:
            s = client_context.wrap_socket(socket.socket(),
                                           server_hostname=hostname)
            s.connect((HOST, server.port))
            s.close()

            self.assertRaises(ValueError, s.read, 1024)
            self.assertRaises(ValueError, s.write, b'hello')

    def test_sendfile(self):
        TEST_DATA = b"x" * 512
        with open(support.TESTFN, 'wb') as f:
            f.write(TEST_DATA)
        self.addCleanup(support.unlink, support.TESTFN)
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        context.verify_mode = ssl.CERT_REQUIRED
        context.load_verify_locations(SIGNING_CA)
        context.load_cert_chain(SIGNED_CERTFILE)
        server = ThreadedEchoServer(context=context, chatty=False)
        with server:
            with context.wrap_socket(socket.socket()) as s:
                s.connect((HOST, server.port))
                with open(support.TESTFN, 'rb') as file:
                    s.sendfile(file)
                    self.assertEqual(s.recv(1024), TEST_DATA)

    def test_session(self):
        client_context, server_context, hostname = testing_context()
        # TODO: sessions aren't compatible with TLSv1.3 yet
        client_context.options |= ssl.OP_NO_TLSv1_3

        # first connection without session
        stats = server_params_test(client_context, server_context,
                                   sni_name=hostname)
        session = stats['session']
        self.assertTrue(session.id)
        self.assertGreater(session.time, 0)
        self.assertGreater(session.timeout, 0)
        self.assertTrue(session.has_ticket)
        if ssl.OPENSSL_VERSION_INFO > (1, 0, 1):
            self.assertGreater(session.ticket_lifetime_hint, 0)
        self.assertFalse(stats['session_reused'])
        sess_stat = server_context.session_stats()
        self.assertEqual(sess_stat['accept'], 1)
        self.assertEqual(sess_stat['hits'], 0)

        # reuse session
        stats = server_params_test(client_context, server_context,
                                   session=session, sni_name=hostname)
        sess_stat = server_context.session_stats()
        self.assertEqual(sess_stat['accept'], 2)
        self.assertEqual(sess_stat['hits'], 1)
        self.assertTrue(stats['session_reused'])
        session2 = stats['session']
        self.assertEqual(session2.id, session.id)
        self.assertEqual(session2, session)
        self.assertIsNot(session2, session)
        self.assertGreaterEqual(session2.time, session.time)
        self.assertGreaterEqual(session2.timeout, session.timeout)

        # another one without session
        stats = server_params_test(client_context, server_context,
                                   sni_name=hostname)
        self.assertFalse(stats['session_reused'])
        session3 = stats['session']
        self.assertNotEqual(session3.id, session.id)
        self.assertNotEqual(session3, session)
        sess_stat = server_context.session_stats()
        self.assertEqual(sess_stat['accept'], 3)
        self.assertEqual(sess_stat['hits'], 1)

        # reuse session again
        stats = server_params_test(client_context, server_context,
                                   session=session, sni_name=hostname)
        self.assertTrue(stats['session_reused'])
        session4 = stats['session']
        self.assertEqual(session4.id, session.id)
        self.assertEqual(session4, session)
        self.assertGreaterEqual(session4.time, session.time)
        self.assertGreaterEqual(session4.timeout, session.timeout)
        sess_stat = server_context.session_stats()
        self.assertEqual(sess_stat['accept'], 4)
        self.assertEqual(sess_stat['hits'], 2)

    def test_session_handling(self):
        client_context, server_context, hostname = testing_context()
        client_context2, _, _ = testing_context()

        # TODO: session reuse does not work with TLSv1.3
        client_context.options |= ssl.OP_NO_TLSv1_3
        client_context2.options |= ssl.OP_NO_TLSv1_3

        server = ThreadedEchoServer(context=server_context, chatty=False)
        with server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                # session is None before handshake
                self.assertEqual(s.session, None)
                self.assertEqual(s.session_reused, None)
                s.connect((HOST, server.port))
                session = s.session
                self.assertTrue(session)
                with self.assertRaises(TypeError) as e:
                    s.session = object
                self.assertEqual(str(e.exception), 'Value is not a SSLSession.')

            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                # cannot set session after handshake
                with self.assertRaises(ValueError) as e:
                    s.session = session
                self.assertEqual(str(e.exception),
                                 'Cannot set session after handshake.')

            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                # can set session before handshake and before the
                # connection was established
                s.session = session
                s.connect((HOST, server.port))
                self.assertEqual(s.session.id, session.id)
                self.assertEqual(s.session, session)
                self.assertEqual(s.session_reused, True)

            with client_context2.wrap_socket(socket.socket(),
                                             server_hostname=hostname) as s:
                # cannot re-use session with a different SSLContext
                with self.assertRaises(ValueError) as e:
                    s.session = session
                    s.connect((HOST, server.port))
                self.assertEqual(str(e.exception),
                                 'Session refers to a different SSLContext.')


def test_main(verbose=False):
    if support.verbose:
        import warnings
        plats = {
            'Linux': platform.linux_distribution,
            'Mac': platform.mac_ver,
            'Windows': platform.win32_ver,
        }
        with warnings.catch_warnings():
            warnings.filterwarnings(
                'ignore',
                r'dist\(\) and linux_distribution\(\) '
                'functions are deprecated .*',
                PendingDeprecationWarning,
            )
            for name, func in plats.items():
                plat = func()
                if plat and plat[0]:
                    plat = '%s %r' % (name, plat)
                    break
            else:
                plat = repr(platform.platform())
        print("test_ssl: testing with %r %r" %
            (ssl.OPENSSL_VERSION, ssl.OPENSSL_VERSION_INFO))
        print("          under %s" % plat)
        print("          HAS_SNI = %r" % ssl.HAS_SNI)
        print("          OP_ALL = 0x%8x" % ssl.OP_ALL)
        try:
            print("          OP_NO_TLSv1_1 = 0x%8x" % ssl.OP_NO_TLSv1_1)
        except AttributeError:
            pass

    for filename in [
        CERTFILE, BYTES_CERTFILE,
        ONLYCERT, ONLYKEY, BYTES_ONLYCERT, BYTES_ONLYKEY,
        SIGNED_CERTFILE, SIGNED_CERTFILE2, SIGNING_CA,
        BADCERT, BADKEY, EMPTYCERT]:
        if not os.path.exists(filename):
            raise support.TestFailed("Can't read certificate file %r" % filename)

    tests = [
        ContextTests, BasicSocketTests, SSLErrorTests, MemoryBIOTests,
        SSLObjectTests, SimpleBackgroundTests, ThreadedTests,
    ]

    if support.is_resource_enabled('network'):
        tests.append(NetworkedTests)

    thread_info = support.threading_setup()
    try:
        support.run_unittest(*tests)
    finally:
        support.threading_cleanup(*thread_info)

if __name__ == "__main__":
    test_main()