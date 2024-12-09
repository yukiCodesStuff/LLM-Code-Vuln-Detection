        def fail(cert, hostname):
            self.assertRaises(ssl.CertificateError,
                              ssl.match_hostname, cert, hostname)

        cert = {'subject': ((('commonName', 'example.com'),),)}
        ok(cert, 'example.com')
        ok(cert, 'ExAmple.cOm')
        fail(cert, 'www.example.com')
        fail(cert, '.example.com')
        fail(cert, 'example.org')
        fail(cert, 'exampleXcom')

        cert = {'subject': ((('commonName', '*.a.com'),),)}
        ok(cert, 'foo.a.com')
        fail(cert, 'bar.foo.a.com')
        fail(cert, 'a.com')
        fail(cert, 'Xa.com')
        fail(cert, '.a.com')

        cert = {'subject': ((('commonName', 'a.*.com'),),)}
        ok(cert, 'a.foo.com')
        fail(cert, 'a..com')
        fail(cert, 'a.com')

        cert = {'subject': ((('commonName', 'f*.com'),),)}
        ok(cert, 'foo.com')
        ok(cert, 'f.com')
        fail(cert, 'bar.com')
        fail(cert, 'foo.a.com')
        fail(cert, 'bar.foo.com')

        # Slightly fake real-world example
        cert = {'notAfter': 'Jun 26 21:41:46 2011 GMT',
                'subject': ((('commonName', 'linuxfrz.org'),),),
                'subjectAltName': (('DNS', 'linuxfr.org'),
                                   ('DNS', 'linuxfr.com'),
                                   ('othername', '<unsupported>'))}
        ok(cert, 'linuxfr.org')
        ok(cert, 'linuxfr.com')
        # Not a "DNS" entry
        fail(cert, '<unsupported>')
        # When there is a subjectAltName, commonName isn't used
        fail(cert, 'linuxfrz.org')

        # A pristine real-world example
        cert = {'notAfter': 'Dec 18 23:59:59 2011 GMT',
                'subject': ((('countryName', 'US'),),
                            (('stateOrProvinceName', 'California'),),
                            (('localityName', 'Mountain View'),),
                            (('organizationName', 'Google Inc'),),
                            (('commonName', 'mail.google.com'),))}
        ok(cert, 'mail.google.com')
        fail(cert, 'gmail.com')
        # Only commonName is considered
        fail(cert, 'California')

        # Neither commonName nor subjectAltName
        cert = {'notAfter': 'Dec 18 23:59:59 2011 GMT',
                'subject': ((('countryName', 'US'),),
                            (('stateOrProvinceName', 'California'),),
                            (('localityName', 'Mountain View'),),
                            (('organizationName', 'Google Inc'),))}
        fail(cert, 'mail.google.com')

        # No DNS entry in subjectAltName but a commonName
        cert = {'notAfter': 'Dec 18 23:59:59 2099 GMT',
                'subject': ((('countryName', 'US'),),
                            (('stateOrProvinceName', 'California'),),
                            (('localityName', 'Mountain View'),),
                            (('commonName', 'mail.google.com'),)),
                'subjectAltName': (('othername', 'blabla'), )}
        ok(cert, 'mail.google.com')

        # No DNS entry subjectAltName and no commonName
        cert = {'notAfter': 'Dec 18 23:59:59 2099 GMT',
                'subject': ((('countryName', 'US'),),
                            (('stateOrProvinceName', 'California'),),
                            (('localityName', 'Mountain View'),),
                            (('organizationName', 'Google Inc'),)),
                'subjectAltName': (('othername', 'blabla'),)}
        fail(cert, 'google.com')

        # Empty cert / no cert
        self.assertRaises(ValueError, ssl.match_hostname, None, 'example.com')
        self.assertRaises(ValueError, ssl.match_hostname, {}, 'example.com')

    def test_server_side(self):
        # server_hostname doesn't work for server sockets
        ctx = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
        with socket.socket() as sock:
            self.assertRaises(ValueError, ctx.wrap_socket, sock, True,
                              server_hostname="some.hostname")

    def test_unknown_channel_binding(self):
        # should raise ValueError for unknown type
        s = socket.socket(socket.AF_INET)
        with ssl.wrap_socket(s) as ss:
            with self.assertRaises(ValueError):
                ss.get_channel_binding("unknown-type")

    @unittest.skipUnless("tls-unique" in ssl.CHANNEL_BINDING_TYPES,
                         "'tls-unique' channel binding not available")
    def test_tls_unique_channel_binding(self):
        # unconnected should return None for known type
        s = socket.socket(socket.AF_INET)
        with ssl.wrap_socket(s) as ss:
            self.assertIsNone(ss.get_channel_binding("tls-unique"))
        # the same for server-side
        s = socket.socket(socket.AF_INET)
        with ssl.wrap_socket(s, server_side=True, certfile=CERTFILE) as ss:
            self.assertIsNone(ss.get_channel_binding("tls-unique"))

    def test_dealloc_warn(self):
        ss = ssl.wrap_socket(socket.socket(socket.AF_INET))
        r = repr(ss)
        with self.assertWarns(ResourceWarning) as cm:
            ss = None
            support.gc_collect()
        self.assertIn(r, str(cm.warning.args[0]))

class ContextTests(unittest.TestCase):

    @skip_if_broken_ubuntu_ssl
    def test_constructor(self):
        if hasattr(ssl, 'PROTOCOL_SSLv2'):
            ctx = ssl.SSLContext(ssl.PROTOCOL_SSLv2)
        ctx = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
        ctx = ssl.SSLContext(ssl.PROTOCOL_SSLv3)
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        self.assertRaises(TypeError, ssl.SSLContext)
        self.assertRaises(ValueError, ssl.SSLContext, -1)
        self.assertRaises(ValueError, ssl.SSLContext, 42)

    @skip_if_broken_ubuntu_ssl
    def test_protocol(self):
        for proto in PROTOCOLS:
            ctx = ssl.SSLContext(proto)
            self.assertEqual(ctx.protocol, proto)

    def test_ciphers(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        ctx.set_ciphers("ALL")
        ctx.set_ciphers("DEFAULT")
        with self.assertRaisesRegex(ssl.SSLError, "No cipher can be selected"):
            ctx.set_ciphers("^$:,;?*'dorothyx")

    @skip_if_broken_ubuntu_ssl
    def test_options(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        # OP_ALL is the default value
        self.assertEqual(ssl.OP_ALL, ctx.options)
        ctx.options |= ssl.OP_NO_SSLv2
        self.assertEqual(ssl.OP_ALL | ssl.OP_NO_SSLv2,
                         ctx.options)
        ctx.options |= ssl.OP_NO_SSLv3
        self.assertEqual(ssl.OP_ALL | ssl.OP_NO_SSLv2 | ssl.OP_NO_SSLv3,
                         ctx.options)
        if can_clear_options():
            ctx.options = (ctx.options & ~ssl.OP_NO_SSLv2) | ssl.OP_NO_TLSv1
            self.assertEqual(ssl.OP_ALL | ssl.OP_NO_TLSv1 | ssl.OP_NO_SSLv3,
                             ctx.options)
            ctx.options = 0
            self.assertEqual(0, ctx.options)
        else:
            with self.assertRaises(ValueError):
                ctx.options = 0

    def test_verify(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        # Default value
        self.assertEqual(ctx.verify_mode, ssl.CERT_NONE)
        ctx.verify_mode = ssl.CERT_OPTIONAL
        self.assertEqual(ctx.verify_mode, ssl.CERT_OPTIONAL)
        ctx.verify_mode = ssl.CERT_REQUIRED
        self.assertEqual(ctx.verify_mode, ssl.CERT_REQUIRED)
        ctx.verify_mode = ssl.CERT_NONE
        self.assertEqual(ctx.verify_mode, ssl.CERT_NONE)
        with self.assertRaises(TypeError):
            ctx.verify_mode = None
        with self.assertRaises(ValueError):
            ctx.verify_mode = 42

    def test_load_cert_chain(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        # Combined key and cert in a single file
        ctx.load_cert_chain(CERTFILE)
        ctx.load_cert_chain(CERTFILE, keyfile=CERTFILE)
        self.assertRaises(TypeError, ctx.load_cert_chain, keyfile=CERTFILE)
        with self.assertRaises(IOError) as cm:
            ctx.load_cert_chain(WRONGCERT)
        self.assertEqual(cm.exception.errno, errno.ENOENT)
        with self.assertRaisesRegex(ssl.SSLError, "PEM lib"):
            ctx.load_cert_chain(BADCERT)
        with self.assertRaisesRegex(ssl.SSLError, "PEM lib"):
            ctx.load_cert_chain(EMPTYCERT)
        # Separate key and cert
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        ctx.load_cert_chain(ONLYCERT, ONLYKEY)
        ctx.load_cert_chain(certfile=ONLYCERT, keyfile=ONLYKEY)
        ctx.load_cert_chain(certfile=BYTES_ONLYCERT, keyfile=BYTES_ONLYKEY)
        with self.assertRaisesRegex(ssl.SSLError, "PEM lib"):
            ctx.load_cert_chain(ONLYCERT)
        with self.assertRaisesRegex(ssl.SSLError, "PEM lib"):
            ctx.load_cert_chain(ONLYKEY)
        with self.assertRaisesRegex(ssl.SSLError, "PEM lib"):
            ctx.load_cert_chain(certfile=ONLYKEY, keyfile=ONLYCERT)
        # Mismatching key and cert
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        with self.assertRaisesRegex(ssl.SSLError, "key values mismatch"):
            ctx.load_cert_chain(SVN_PYTHON_ORG_ROOT_CERT, ONLYKEY)
        # Password protected key and cert
        ctx.load_cert_chain(CERTFILE_PROTECTED, password=KEY_PASSWORD)
        ctx.load_cert_chain(CERTFILE_PROTECTED, password=KEY_PASSWORD.encode())
        ctx.load_cert_chain(CERTFILE_PROTECTED,
                            password=bytearray(KEY_PASSWORD.encode()))
        ctx.load_cert_chain(ONLYCERT, ONLYKEY_PROTECTED, KEY_PASSWORD)
        ctx.load_cert_chain(ONLYCERT, ONLYKEY_PROTECTED, KEY_PASSWORD.encode())
        ctx.load_cert_chain(ONLYCERT, ONLYKEY_PROTECTED,
                            bytearray(KEY_PASSWORD.encode()))
        with self.assertRaisesRegex(TypeError, "should be a string"):
            ctx.load_cert_chain(CERTFILE_PROTECTED, password=True)
        with self.assertRaises(ssl.SSLError):
            ctx.load_cert_chain(CERTFILE_PROTECTED, password="badpass")
        with self.assertRaisesRegex(ValueError, "cannot be longer"):
            # openssl has a fixed limit on the password buffer.
            # PEM_BUFSIZE is generally set to 1kb.
            # Return a string larger than this.
            ctx.load_cert_chain(CERTFILE_PROTECTED, password=b'a' * 102400)
        # Password callback
        def getpass_unicode():
            return KEY_PASSWORD
        def getpass_bytes():
            return KEY_PASSWORD.encode()
        def getpass_bytearray():
            return bytearray(KEY_PASSWORD.encode())
        def getpass_badpass():
            return "badpass"
        def getpass_huge():
            return b'a' * (1024 * 1024)
        def getpass_bad_type():
            return 9
        def getpass_exception():
            raise Exception('getpass error')
        class GetPassCallable:
            def __call__(self):
                return KEY_PASSWORD
            def getpass(self):
                return KEY_PASSWORD
        ctx.load_cert_chain(CERTFILE_PROTECTED, password=getpass_unicode)
        ctx.load_cert_chain(CERTFILE_PROTECTED, password=getpass_bytes)
        ctx.load_cert_chain(CERTFILE_PROTECTED, password=getpass_bytearray)
        ctx.load_cert_chain(CERTFILE_PROTECTED, password=GetPassCallable())
        ctx.load_cert_chain(CERTFILE_PROTECTED,
                            password=GetPassCallable().getpass)
        with self.assertRaises(ssl.SSLError):
            ctx.load_cert_chain(CERTFILE_PROTECTED, password=getpass_badpass)
        with self.assertRaisesRegex(ValueError, "cannot be longer"):
            ctx.load_cert_chain(CERTFILE_PROTECTED, password=getpass_huge)
        with self.assertRaisesRegex(TypeError, "must return a string"):
            ctx.load_cert_chain(CERTFILE_PROTECTED, password=getpass_bad_type)
        with self.assertRaisesRegex(Exception, "getpass error"):
            ctx.load_cert_chain(CERTFILE_PROTECTED, password=getpass_exception)
        # Make sure the password function isn't called if it isn't needed
        ctx.load_cert_chain(CERTFILE, password=getpass_exception)

    def test_load_verify_locations(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        ctx.load_verify_locations(CERTFILE)
        ctx.load_verify_locations(cafile=CERTFILE, capath=None)
        ctx.load_verify_locations(BYTES_CERTFILE)
        ctx.load_verify_locations(cafile=BYTES_CERTFILE, capath=None)
        self.assertRaises(TypeError, ctx.load_verify_locations)
        self.assertRaises(TypeError, ctx.load_verify_locations, None, None)
        with self.assertRaises(IOError) as cm:
            ctx.load_verify_locations(WRONGCERT)
        self.assertEqual(cm.exception.errno, errno.ENOENT)
        with self.assertRaisesRegex(ssl.SSLError, "PEM lib"):
            ctx.load_verify_locations(BADCERT)
        ctx.load_verify_locations(CERTFILE, CAPATH)
        ctx.load_verify_locations(CERTFILE, capath=BYTES_CAPATH)

        # Issue #10989: crash if the second argument type is invalid
        self.assertRaises(TypeError, ctx.load_verify_locations, None, True)

    def test_load_dh_params(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        ctx.load_dh_params(DHFILE)
        if os.name != 'nt':
            ctx.load_dh_params(BYTES_DHFILE)
        self.assertRaises(TypeError, ctx.load_dh_params)
        self.assertRaises(TypeError, ctx.load_dh_params, None)
        with self.assertRaises(FileNotFoundError) as cm:
            ctx.load_dh_params(WRONGCERT)
        self.assertEqual(cm.exception.errno, errno.ENOENT)
        with self.assertRaises(ssl.SSLError) as cm:
            ctx.load_dh_params(CERTFILE)

    @skip_if_broken_ubuntu_ssl
    def test_session_stats(self):
        for proto in PROTOCOLS:
            ctx = ssl.SSLContext(proto)
            self.assertEqual(ctx.session_stats(), {
                'number': 0,
                'connect': 0,
                'connect_good': 0,
                'connect_renegotiate': 0,
                'accept': 0,
                'accept_good': 0,
                'accept_renegotiate': 0,
                'hits': 0,
                'misses': 0,
                'timeouts': 0,
                'cache_full': 0,
            })

    def test_set_default_verify_paths(self):
        # There's not much we can do to test that it acts as expected,
        # so just check it doesn't crash or raise an exception.
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        ctx.set_default_verify_paths()

    @unittest.skipUnless(ssl.HAS_ECDH, "ECDH disabled on this OpenSSL build")
    def test_set_ecdh_curve(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        ctx.set_ecdh_curve("prime256v1")
        ctx.set_ecdh_curve(b"prime256v1")
        self.assertRaises(TypeError, ctx.set_ecdh_curve)
        self.assertRaises(TypeError, ctx.set_ecdh_curve, None)
        self.assertRaises(ValueError, ctx.set_ecdh_curve, "foo")
        self.assertRaises(ValueError, ctx.set_ecdh_curve, b"foo")


class SSLErrorTests(unittest.TestCase):

    def test_str(self):
        # The str() of a SSLError doesn't include the errno
        e = ssl.SSLError(1, "foo")
        self.assertEqual(str(e), "foo")
        self.assertEqual(e.errno, 1)
        # Same for a subclass
        e = ssl.SSLZeroReturnError(1, "foo")
        self.assertEqual(str(e), "foo")
        self.assertEqual(e.errno, 1)

    def test_lib_reason(self):
        # Test the library and reason attributes
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        with self.assertRaises(ssl.SSLError) as cm:
            ctx.load_dh_params(CERTFILE)
        self.assertEqual(cm.exception.library, 'PEM')
        self.assertEqual(cm.exception.reason, 'NO_START_LINE')
        s = str(cm.exception)
        self.assertTrue(s.startswith("[PEM: NO_START_LINE] no start line"), s)

    def test_subclass(self):
        # Check that the appropriate SSLError subclass is raised
        # (this only tests one of them)
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        with socket.socket() as s:
            s.bind(("127.0.0.1", 0))
            s.listen(5)
            c = socket.socket()
            c.connect(s.getsockname())
            c.setblocking(False)
            with ctx.wrap_socket(c, False, do_handshake_on_connect=False) as c:
                with self.assertRaises(ssl.SSLWantReadError) as cm:
                    c.do_handshake()
                s = str(cm.exception)
                self.assertTrue(s.startswith("The operation did not complete (read)"), s)
                # For compatibility
                self.assertEqual(cm.exception.errno, ssl.SSL_ERROR_WANT_READ)


class NetworkedTests(unittest.TestCase):

    def test_connect(self):
        with support.transient_internet("svn.python.org"):
            s = ssl.wrap_socket(socket.socket(socket.AF_INET),
                                cert_reqs=ssl.CERT_NONE)
            try:
                s.connect(("svn.python.org", 443))
                self.assertEqual({}, s.getpeercert())
            finally:
                s.close()

            # this should fail because we have no verification certs
            s = ssl.wrap_socket(socket.socket(socket.AF_INET),
                                cert_reqs=ssl.CERT_REQUIRED)
            self.assertRaisesRegex(ssl.SSLError, "certificate verify failed",
                                   s.connect, ("svn.python.org", 443))
            s.close()

            # this should succeed because we specify the root cert
            s = ssl.wrap_socket(socket.socket(socket.AF_INET),
                                cert_reqs=ssl.CERT_REQUIRED,
                                ca_certs=SVN_PYTHON_ORG_ROOT_CERT)
            try:
                s.connect(("svn.python.org", 443))
                self.assertTrue(s.getpeercert())
            finally:
                s.close()

    def test_connect_ex(self):
        # Issue #11326: check connect_ex() implementation
        with support.transient_internet("svn.python.org"):
            s = ssl.wrap_socket(socket.socket(socket.AF_INET),
                                cert_reqs=ssl.CERT_REQUIRED,
                                ca_certs=SVN_PYTHON_ORG_ROOT_CERT)
            try:
                self.assertEqual(0, s.connect_ex(("svn.python.org", 443)))
                self.assertTrue(s.getpeercert())
            finally:
                s.close()

    def test_non_blocking_connect_ex(self):
        # Issue #11326: non-blocking connect_ex() should allow handshake
        # to proceed after the socket gets ready.
        with support.transient_internet("svn.python.org"):
            s = ssl.wrap_socket(socket.socket(socket.AF_INET),
                                cert_reqs=ssl.CERT_REQUIRED,
                                ca_certs=SVN_PYTHON_ORG_ROOT_CERT,
                                do_handshake_on_connect=False)
            try:
                s.setblocking(False)
                rc = s.connect_ex(('svn.python.org', 443))
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
            finally:
                s.close()

    def test_timeout_connect_ex(self):
        # Issue #12065: on a timeout, connect_ex() should return the original
        # errno (mimicking the behaviour of non-SSL sockets).
        with support.transient_internet("svn.python.org"):
            s = ssl.wrap_socket(socket.socket(socket.AF_INET),
                                cert_reqs=ssl.CERT_REQUIRED,
                                ca_certs=SVN_PYTHON_ORG_ROOT_CERT,
                                do_handshake_on_connect=False)
            try:
                s.settimeout(0.0000001)
                rc = s.connect_ex(('svn.python.org', 443))
                if rc == 0:
                    self.skipTest("svn.python.org responded too quickly")
                self.assertIn(rc, (errno.EAGAIN, errno.EWOULDBLOCK))
            finally:
                s.close()

    def test_connect_ex_error(self):
        with support.transient_internet("svn.python.org"):
            s = ssl.wrap_socket(socket.socket(socket.AF_INET),
                                cert_reqs=ssl.CERT_REQUIRED,
                                ca_certs=SVN_PYTHON_ORG_ROOT_CERT)
            try:
                self.assertEqual(errno.ECONNREFUSED,
                                 s.connect_ex(("svn.python.org", 444)))
            finally:
                s.close()

    def test_connect_with_context(self):
        with support.transient_internet("svn.python.org"):
            # Same as test_connect, but with a separately created context
            ctx = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
            s = ctx.wrap_socket(socket.socket(socket.AF_INET))
            s.connect(("svn.python.org", 443))
            try:
                self.assertEqual({}, s.getpeercert())
            finally:
                s.close()
            # Same with a server hostname
            s = ctx.wrap_socket(socket.socket(socket.AF_INET),
                                server_hostname="svn.python.org")
            if ssl.HAS_SNI:
                s.connect(("svn.python.org", 443))
                s.close()
            else:
                self.assertRaises(ValueError, s.connect, ("svn.python.org", 443))
            # This should fail because we have no verification certs
            ctx.verify_mode = ssl.CERT_REQUIRED
            s = ctx.wrap_socket(socket.socket(socket.AF_INET))
            self.assertRaisesRegex(ssl.SSLError, "certificate verify failed",
                                    s.connect, ("svn.python.org", 443))
            s.close()
            # This should succeed because we specify the root cert
            ctx.load_verify_locations(SVN_PYTHON_ORG_ROOT_CERT)
            s = ctx.wrap_socket(socket.socket(socket.AF_INET))
            s.connect(("svn.python.org", 443))
            try:
                cert = s.getpeercert()
                self.assertTrue(cert)
            finally:
                s.close()

    def test_connect_capath(self):
        # Verify server certificates using the `capath` argument
        # NOTE: the subject hashing algorithm has been changed between
        # OpenSSL 0.9.8n and 1.0.0, as a result the capath directory must
        # contain both versions of each certificate (same content, different
        # filename) for this test to be portable across OpenSSL releases.
        with support.transient_internet("svn.python.org"):
            ctx = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
            ctx.verify_mode = ssl.CERT_REQUIRED
            ctx.load_verify_locations(capath=CAPATH)
            s = ctx.wrap_socket(socket.socket(socket.AF_INET))
            s.connect(("svn.python.org", 443))
            try:
                cert = s.getpeercert()
                self.assertTrue(cert)
            finally:
                s.close()
            # Same with a bytes `capath` argument
            ctx = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
            ctx.verify_mode = ssl.CERT_REQUIRED
            ctx.load_verify_locations(capath=BYTES_CAPATH)
            s = ctx.wrap_socket(socket.socket(socket.AF_INET))
            s.connect(("svn.python.org", 443))
            try:
                cert = s.getpeercert()
                self.assertTrue(cert)
            finally:
                s.close()

    @unittest.skipIf(os.name == "nt", "Can't use a socket as a file under Windows")
    def test_makefile_close(self):
        # Issue #5238: creating a file-like object with makefile() shouldn't
        # delay closing the underlying "real socket" (here tested with its
        # file descriptor, hence skipping the test under Windows).
        with support.transient_internet("svn.python.org"):
            ss = ssl.wrap_socket(socket.socket(socket.AF_INET))
            ss.connect(("svn.python.org", 443))
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
        with support.transient_internet("svn.python.org"):
            s = socket.socket(socket.AF_INET)
            s.connect(("svn.python.org", 443))
            s.setblocking(False)
            s = ssl.wrap_socket(s,
                                cert_reqs=ssl.CERT_NONE,
                                do_handshake_on_connect=False)
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
            s.close()
            if support.verbose:
                sys.stdout.write("\nNeeded %d calls to do_handshake() to establish session.\n" % count)

    def test_get_server_certificate(self):
        def _test_get_server_certificate(host, port, cert=None):
            with support.transient_internet(host):
                pem = ssl.get_server_certificate((host, port))
                if not pem:
                    self.fail("No server certificate on %s:%s!" % (host, port))

                try:
                    pem = ssl.get_server_certificate((host, port), ca_certs=CERTFILE)
                except ssl.SSLError as x:
                    #should fail
                    if support.verbose:
                        sys.stdout.write("%s\n" % x)
                else:
                    self.fail("Got server certificate %s for %s:%s!" % (pem, host, port))

                pem = ssl.get_server_certificate((host, port), ca_certs=cert)
                if not pem:
                    self.fail("No server certificate on %s:%s!" % (host, port))
                if support.verbose:
                    sys.stdout.write("\nVerified certificate for %s:%s is\n%s\n" % (host, port ,pem))

        _test_get_server_certificate('svn.python.org', 443, SVN_PYTHON_ORG_ROOT_CERT)
        if support.IPV6_ENABLED:
            _test_get_server_certificate('ipv6.google.com', 443)

    def test_ciphers(self):
        remote = ("svn.python.org", 443)
        with support.transient_internet(remote[0]):
            with ssl.wrap_socket(socket.socket(socket.AF_INET),
                                 cert_reqs=ssl.CERT_NONE, ciphers="ALL") as s:
                s.connect(remote)
            with ssl.wrap_socket(socket.socket(socket.AF_INET),
                                 cert_reqs=ssl.CERT_NONE, ciphers="DEFAULT") as s:
                s.connect(remote)
            # Error checking can happen at instantiation or when connecting
            with self.assertRaisesRegex(ssl.SSLError, "No cipher can be selected"):
                with socket.socket(socket.AF_INET) as sock:
                    s = ssl.wrap_socket(sock,
                                        cert_reqs=ssl.CERT_NONE, ciphers="^$:,;?*'dorothyx")
                    s.connect(remote)

    def test_algorithms(self):
        # Issue #8484: all algorithms should be available when verifying a
        # certificate.
        # SHA256 was added in OpenSSL 0.9.8
        if ssl.OPENSSL_VERSION_INFO < (0, 9, 8, 0, 15):
            self.skipTest("SHA256 not available on %r" % ssl.OPENSSL_VERSION)
        # sha256.tbs-internet.com needs SNI to use the correct certificate
        if not ssl.HAS_SNI:
            self.skipTest("SNI needed for this test")
        # https://sha2.hboeck.de/ was used until 2011-01-08 (no route to host)
        remote = ("sha256.tbs-internet.com", 443)
        sha256_cert = os.path.join(os.path.dirname(__file__), "sha256.pem")
        with support.transient_internet("sha256.tbs-internet.com"):
            ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
            ctx.verify_mode = ssl.CERT_REQUIRED
            ctx.load_verify_locations(sha256_cert)
            s = ctx.wrap_socket(socket.socket(socket.AF_INET),
                                server_hostname="sha256.tbs-internet.com")
            try:
                s.connect(remote)
                if support.verbose:
                    sys.stdout.write("\nCipher with %r is %r\n" %
                                     (remote, s.cipher()))
                    sys.stdout.write("Certificate is:\n%s\n" %
                                     pprint.pformat(s.getpeercert()))
            finally:
                s.close()


try:
    import threading
except ImportError:
    _have_threads = False
else:
    _have_threads = True

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
                    self.server.selected_protocols.append(self.sslconn.selected_npn_protocol())
                except (ssl.SSLError, ConnectionResetError) as e:
                    # We treat ConnectionResetError as though it were an
                    # SSLError - OpenSSL on Ubuntu abruptly closes the
                    # connection when asked to use an unsupported protocol.
                    #
                    # XXX Various errors can have happened here, for example
                    # a mismatching protocol version, an invalid certificate,
                    # or a low-level bug. This should be made more discriminating.
                    self.server.conn_errors.append(e)
                    if self.server.chatty:
                        handle_error("\n server:  bad connection attempt from " + repr(self.addr) + ":\n")
                    self.running = False
                    self.server.stop()
                    self.close()
                    return False
                else:
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
                    except socket.error:
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
                     npn_protocols=None, ciphers=None, context=None):
            if context:
                self.context = context
            else:
                self.context = ssl.SSLContext(ssl_version
                                              if ssl_version is not None
                                              else ssl.PROTOCOL_TLSv1)
                self.context.verify_mode = (certreqs if certreqs is not None
                                            else ssl.CERT_NONE)
                if cacerts:
                    self.context.load_verify_locations(cacerts)
                if certificate:
                    self.context.load_cert_chain(certificate)
                if npn_protocols:
                    self.context.set_npn_protocols(npn_protocols)
                if ciphers:
                    self.context.set_ciphers(ciphers)
            self.chatty = chatty
            self.connectionchatty = connectionchatty
            self.starttls_server = starttls_server
            self.sock = socket.socket()
            self.port = support.bind_port(self.sock)
            self.flag = None
            self.active = False
            self.selected_protocols = []
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
            self.sock.listen(5)
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

            class ConnectionHandler (asyncore.dispatcher_with_send):

                def __init__(self, conn, certfile):
                    self.socket = ssl.wrap_socket(conn, server_side=True,
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
                    except socket.error as err:
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

    def bad_cert_test(certfile):
        """
        Launch a server with CERT_REQUIRED, and check that trying to
        connect to it with the given client certificate fails.
        """
        server = ThreadedEchoServer(CERTFILE,
                                    certreqs=ssl.CERT_REQUIRED,
                                    cacerts=CERTFILE, chatty=False,
                                    connectionchatty=False)
        with server:
            try:
                with socket.socket() as sock:
                    s = ssl.wrap_socket(sock,
                                        certfile=certfile,
                                        ssl_version=ssl.PROTOCOL_TLSv1)
                    s.connect((HOST, server.port))
            except ssl.SSLError as x:
                if support.verbose:
                    sys.stdout.write("\nSSLError is %s\n" % x.args[1])
            except socket.error as x:
                if support.verbose:
                    sys.stdout.write("\nsocket.error is %s\n" % x.args[1])
            except IOError as x:
                if x.errno != errno.ENOENT:
                    raise
                if support.verbose:
                    sys.stdout.write("\IOError is %s\n" % str(x))
            else:
                raise AssertionError("Use of invalid cert should have failed!")

    def server_params_test(client_context, server_context, indata=b"FOO\n",
                           chatty=True, connectionchatty=False):
        """
        Launch a server, connect a client to it and try various reads
        and writes.
        """
        stats = {}