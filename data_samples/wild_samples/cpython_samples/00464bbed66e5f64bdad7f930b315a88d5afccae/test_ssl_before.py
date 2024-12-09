            with self.subTest(version=version):
                ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
                with self.assertWarns(DeprecationWarning) as cm:
                    ctx.minimum_version = version
                version_text = '%s.%s' % (version.__class__.__name__, version.name)
                self.assertEqual(
                    f'ssl.{version_text} is deprecated',
                    str(cm.warning)
                )

    @ignore_deprecation
    def test_errors_sslwrap(self):
        sock = socket.socket()
        self.assertRaisesRegex(ValueError,
                        "certfile must be specified",
                        ssl.wrap_socket, sock, keyfile=CERTFILE)
        self.assertRaisesRegex(ValueError,
                        "certfile must be specified for server-side operations",
                        ssl.wrap_socket, sock, server_side=True)
        self.assertRaisesRegex(ValueError,
                        "certfile must be specified for server-side operations",
                         ssl.wrap_socket, sock, server_side=True, certfile="")
        with ssl.wrap_socket(sock, server_side=True, certfile=CERTFILE) as s:
            self.assertRaisesRegex(ValueError, "can't connect in server-side mode",
                                     s.connect, (HOST, 8080))
        with self.assertRaises(OSError) as cm:
            with socket.socket() as sock:
                ssl.wrap_socket(sock, certfile=NONEXISTINGCERT)
        self.assertEqual(cm.exception.errno, errno.ENOENT)
        with self.assertRaises(OSError) as cm:
            with socket.socket() as sock:
                ssl.wrap_socket(sock,
                    certfile=CERTFILE, keyfile=NONEXISTINGCERT)
        self.assertEqual(cm.exception.errno, errno.ENOENT)
        with self.assertRaises(OSError) as cm:
            with socket.socket() as sock:
                ssl.wrap_socket(sock,
                    certfile=NONEXISTINGCERT, keyfile=NONEXISTINGCERT)
        self.assertEqual(cm.exception.errno, errno.ENOENT)

    def bad_cert_test(self, certfile):
        """Check that trying to use the given client certificate fails"""
        certfile = os.path.join(os.path.dirname(__file__) or os.curdir,
                                   certfile)
        sock = socket.socket()
        self.addCleanup(sock.close)
        with self.assertRaises(ssl.SSLError):
            test_wrap_socket(sock,
                             certfile=certfile)

    def test_empty_cert(self):
        """Wrapping with an empty cert file"""
        self.bad_cert_test("nullcert.pem")

    def test_malformed_cert(self):
        """Wrapping with a badly formatted certificate (syntax error)"""
        self.bad_cert_test("badcert.pem")

    def test_malformed_key(self):
        """Wrapping with a badly formatted key (syntax error)"""
        self.bad_cert_test("badkey.pem")

    def test_server_side(self):
        # server_hostname doesn't work for server sockets
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        with socket.socket() as sock:
            self.assertRaises(ValueError, ctx.wrap_socket, sock, True,
                              server_hostname="some.hostname")

    def test_unknown_channel_binding(self):
        # should raise ValueError for unknown type
        s = socket.create_server(('127.0.0.1', 0))
        c = socket.socket(socket.AF_INET)
        c.connect(s.getsockname())
        with test_wrap_socket(c, do_handshake_on_connect=False) as ss:
            with self.assertRaises(ValueError):
                ss.get_channel_binding("unknown-type")
        s.close()

    @unittest.skipUnless("tls-unique" in ssl.CHANNEL_BINDING_TYPES,
                         "'tls-unique' channel binding not available")
    def test_tls_unique_channel_binding(self):
        # unconnected should return None for known type
        s = socket.socket(socket.AF_INET)
        with test_wrap_socket(s) as ss:
            self.assertIsNone(ss.get_channel_binding("tls-unique"))
        # the same for server-side
        s = socket.socket(socket.AF_INET)
        with test_wrap_socket(s, server_side=True, certfile=CERTFILE) as ss:
            self.assertIsNone(ss.get_channel_binding("tls-unique"))

    def test_dealloc_warn(self):
        ss = test_wrap_socket(socket.socket(socket.AF_INET))
        r = repr(ss)
        with self.assertWarns(ResourceWarning) as cm:
            ss = None
            support.gc_collect()
        self.assertIn(r, str(cm.warning.args[0]))

    def test_get_default_verify_paths(self):
        paths = ssl.get_default_verify_paths()
        self.assertEqual(len(paths), 6)
        self.assertIsInstance(paths, ssl.DefaultVerifyPaths)

        with os_helper.EnvironmentVarGuard() as env:
            env["SSL_CERT_DIR"] = CAPATH
            env["SSL_CERT_FILE"] = CERTFILE
            paths = ssl.get_default_verify_paths()
            self.assertEqual(paths.cafile, CERTFILE)
            self.assertEqual(paths.capath, CAPATH)

    @unittest.skipUnless(sys.platform == "win32", "Windows specific")
    def test_enum_certificates(self):
        self.assertTrue(ssl.enum_certificates("CA"))
        self.assertTrue(ssl.enum_certificates("ROOT"))

        self.assertRaises(TypeError, ssl.enum_certificates)
        self.assertRaises(WindowsError, ssl.enum_certificates, "")

        trust_oids = set()
        for storename in ("CA", "ROOT"):
            store = ssl.enum_certificates(storename)
            self.assertIsInstance(store, list)
            for element in store:
                self.assertIsInstance(element, tuple)
                self.assertEqual(len(element), 3)
                cert, enc, trust = element
                self.assertIsInstance(cert, bytes)
                self.assertIn(enc, {"x509_asn", "pkcs_7_asn"})
                self.assertIsInstance(trust, (frozenset, set, bool))
                if isinstance(trust, (frozenset, set)):
                    trust_oids.update(trust)

        serverAuth = "1.3.6.1.5.5.7.3.1"
        self.assertIn(serverAuth, trust_oids)

    @unittest.skipUnless(sys.platform == "win32", "Windows specific")
    def test_enum_crls(self):
        self.assertTrue(ssl.enum_crls("CA"))
        self.assertRaises(TypeError, ssl.enum_crls)
        self.assertRaises(WindowsError, ssl.enum_crls, "")

        crls = ssl.enum_crls("CA")
        self.assertIsInstance(crls, list)
        for element in crls:
            self.assertIsInstance(element, tuple)
            self.assertEqual(len(element), 2)
            self.assertIsInstance(element[0], bytes)
            self.assertIn(element[1], {"x509_asn", "pkcs_7_asn"})


    def test_asn1object(self):
        expected = (129, 'serverAuth', 'TLS Web Server Authentication',
                    '1.3.6.1.5.5.7.3.1')

        val = ssl._ASN1Object('1.3.6.1.5.5.7.3.1')
        self.assertEqual(val, expected)
        self.assertEqual(val.nid, 129)
        self.assertEqual(val.shortname, 'serverAuth')
        self.assertEqual(val.longname, 'TLS Web Server Authentication')
        self.assertEqual(val.oid, '1.3.6.1.5.5.7.3.1')
        self.assertIsInstance(val, ssl._ASN1Object)
        self.assertRaises(ValueError, ssl._ASN1Object, 'serverAuth')

        val = ssl._ASN1Object.fromnid(129)
        self.assertEqual(val, expected)
        self.assertIsInstance(val, ssl._ASN1Object)
        self.assertRaises(ValueError, ssl._ASN1Object.fromnid, -1)
        with self.assertRaisesRegex(ValueError, "unknown NID 100000"):
            ssl._ASN1Object.fromnid(100000)
        for i in range(1000):
            try:
                obj = ssl._ASN1Object.fromnid(i)
            except ValueError:
                pass
            else:
                self.assertIsInstance(obj.nid, int)
                self.assertIsInstance(obj.shortname, str)
                self.assertIsInstance(obj.longname, str)
                self.assertIsInstance(obj.oid, (str, type(None)))

        val = ssl._ASN1Object.fromname('TLS Web Server Authentication')
        self.assertEqual(val, expected)
        self.assertIsInstance(val, ssl._ASN1Object)
        self.assertEqual(ssl._ASN1Object.fromname('serverAuth'), expected)
        self.assertEqual(ssl._ASN1Object.fromname('1.3.6.1.5.5.7.3.1'),
                         expected)
        with self.assertRaisesRegex(ValueError, "unknown object 'serverauth'"):
            ssl._ASN1Object.fromname('serverauth')

    def test_purpose_enum(self):
        val = ssl._ASN1Object('1.3.6.1.5.5.7.3.1')
        self.assertIsInstance(ssl.Purpose.SERVER_AUTH, ssl._ASN1Object)
        self.assertEqual(ssl.Purpose.SERVER_AUTH, val)
        self.assertEqual(ssl.Purpose.SERVER_AUTH.nid, 129)
        self.assertEqual(ssl.Purpose.SERVER_AUTH.shortname, 'serverAuth')
        self.assertEqual(ssl.Purpose.SERVER_AUTH.oid,
                              '1.3.6.1.5.5.7.3.1')

        val = ssl._ASN1Object('1.3.6.1.5.5.7.3.2')
        self.assertIsInstance(ssl.Purpose.CLIENT_AUTH, ssl._ASN1Object)
        self.assertEqual(ssl.Purpose.CLIENT_AUTH, val)
        self.assertEqual(ssl.Purpose.CLIENT_AUTH.nid, 130)
        self.assertEqual(ssl.Purpose.CLIENT_AUTH.shortname, 'clientAuth')
        self.assertEqual(ssl.Purpose.CLIENT_AUTH.oid,
                              '1.3.6.1.5.5.7.3.2')

    def test_unsupported_dtls(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.addCleanup(s.close)
        with self.assertRaises(NotImplementedError) as cx:
            test_wrap_socket(s, cert_reqs=ssl.CERT_NONE)
        self.assertEqual(str(cx.exception), "only stream sockets are supported")
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        with self.assertRaises(NotImplementedError) as cx:
            ctx.wrap_socket(s)
        self.assertEqual(str(cx.exception), "only stream sockets are supported")

    def cert_time_ok(self, timestring, timestamp):
        self.assertEqual(ssl.cert_time_to_seconds(timestring), timestamp)

    def cert_time_fail(self, timestring):
        with self.assertRaises(ValueError):
            ssl.cert_time_to_seconds(timestring)

    @unittest.skipUnless(utc_offset(),
                         'local time needs to be different from UTC')
    def test_cert_time_to_seconds_timezone(self):
        # Issue #19940: ssl.cert_time_to_seconds() returns wrong
        #               results if local timezone is not UTC
        self.cert_time_ok("May  9 00:00:00 2007 GMT", 1178668800.0)
        self.cert_time_ok("Jan  5 09:34:43 2018 GMT", 1515144883.0)

    def test_cert_time_to_seconds(self):
        timestring = "Jan  5 09:34:43 2018 GMT"
        ts = 1515144883.0
        self.cert_time_ok(timestring, ts)
        # accept keyword parameter, assert its name
        self.assertEqual(ssl.cert_time_to_seconds(cert_time=timestring), ts)
        # accept both %e and %d (space or zero generated by strftime)
        self.cert_time_ok("Jan 05 09:34:43 2018 GMT", ts)
        # case-insensitive
        self.cert_time_ok("JaN  5 09:34:43 2018 GmT", ts)
        self.cert_time_fail("Jan  5 09:34 2018 GMT")     # no seconds
        self.cert_time_fail("Jan  5 09:34:43 2018")      # no GMT
        self.cert_time_fail("Jan  5 09:34:43 2018 UTC")  # not GMT timezone
        self.cert_time_fail("Jan 35 09:34:43 2018 GMT")  # invalid day
        self.cert_time_fail("Jon  5 09:34:43 2018 GMT")  # invalid month
        self.cert_time_fail("Jan  5 24:00:00 2018 GMT")  # invalid hour
        self.cert_time_fail("Jan  5 09:60:43 2018 GMT")  # invalid minute

        newyear_ts = 1230768000.0
        # leap seconds
        self.cert_time_ok("Dec 31 23:59:60 2008 GMT", newyear_ts)
        # same timestamp
        self.cert_time_ok("Jan  1 00:00:00 2009 GMT", newyear_ts)

        self.cert_time_ok("Jan  5 09:34:59 2018 GMT", 1515144899)
        #  allow 60th second (even if it is not a leap second)
        self.cert_time_ok("Jan  5 09:34:60 2018 GMT", 1515144900)
        #  allow 2nd leap second for compatibility with time.strptime()
        self.cert_time_ok("Jan  5 09:34:61 2018 GMT", 1515144901)
        self.cert_time_fail("Jan  5 09:34:62 2018 GMT")  # invalid seconds

        # no special treatment for the special value:
        #   99991231235959Z (rfc 5280)
        self.cert_time_ok("Dec 31 23:59:59 9999 GMT", 253402300799.0)

    @support.run_with_locale('LC_ALL', '')
    def test_cert_time_to_seconds_locale(self):
        # `cert_time_to_seconds()` should be locale independent

        def local_february_name():
            return time.strftime('%b', (1, 2, 3, 4, 5, 6, 0, 0, 0))

        if local_february_name().lower() == 'feb':
            self.skipTest("locale-specific month name needs to be "
                          "different from C locale")

        # locale-independent
        self.cert_time_ok("Feb  9 00:00:00 2007 GMT", 1170979200.0)
        self.cert_time_fail(local_february_name() + "  9 00:00:00 2007 GMT")

    def test_connect_ex_error(self):
        server = socket.socket(socket.AF_INET)
        self.addCleanup(server.close)
        port = socket_helper.bind_port(server)  # Reserve port but don't listen
        s = test_wrap_socket(socket.socket(socket.AF_INET),
                            cert_reqs=ssl.CERT_REQUIRED)
        self.addCleanup(s.close)
        rc = s.connect_ex((HOST, port))
        # Issue #19919: Windows machines or VMs hosted on Windows
        # machines sometimes return EWOULDBLOCK.
        errors = (
            errno.ECONNREFUSED, errno.EHOSTUNREACH, errno.ETIMEDOUT,
            errno.EWOULDBLOCK,
        )
        self.assertIn(rc, errors)

    def test_read_write_zero(self):
        # empty reads and writes now work, bpo-42854, bpo-31711
        client_context, server_context, hostname = testing_context()
        server = ThreadedEchoServer(context=server_context)
        with server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                s.connect((HOST, server.port))
                self.assertEqual(s.recv(0), b"")
                self.assertEqual(s.send(b""), 0)


class ContextTests(unittest.TestCase):

    def test_constructor(self):
        for protocol in PROTOCOLS:
            if has_tls_protocol(protocol):
                with warnings_helper.check_warnings():
                    ctx = ssl.SSLContext(protocol)
                self.assertEqual(ctx.protocol, protocol)
        with warnings_helper.check_warnings():
            ctx = ssl.SSLContext()
        self.assertEqual(ctx.protocol, ssl.PROTOCOL_TLS)
        self.assertRaises(ValueError, ssl.SSLContext, -1)
        self.assertRaises(ValueError, ssl.SSLContext, 42)

    def test_ciphers(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        ctx.set_ciphers("ALL")
        ctx.set_ciphers("DEFAULT")
        with self.assertRaisesRegex(ssl.SSLError, "No cipher can be selected"):
            ctx.set_ciphers("^$:,;?*'dorothyx")

    @unittest.skipUnless(PY_SSL_DEFAULT_CIPHERS == 1,
                         "Test applies only to Python default ciphers")
    def test_python_ciphers(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        ciphers = ctx.get_ciphers()
        for suite in ciphers:
            name = suite['name']
            self.assertNotIn("PSK", name)
            self.assertNotIn("SRP", name)
            self.assertNotIn("MD5", name)
            self.assertNotIn("RC4", name)
            self.assertNotIn("3DES", name)

    def test_get_ciphers(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        ctx.set_ciphers('AESGCM')
        names = set(d['name'] for d in ctx.get_ciphers())
        self.assertIn('AES256-GCM-SHA384', names)
        self.assertIn('AES128-GCM-SHA256', names)

    def test_options(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        # OP_ALL | OP_NO_SSLv2 | OP_NO_SSLv3 is the default value
        default = (ssl.OP_ALL | ssl.OP_NO_SSLv2 | ssl.OP_NO_SSLv3)
        # SSLContext also enables these by default
        default |= (OP_NO_COMPRESSION | OP_CIPHER_SERVER_PREFERENCE |
                    OP_SINGLE_DH_USE | OP_SINGLE_ECDH_USE |
                    OP_ENABLE_MIDDLEBOX_COMPAT |
                    OP_IGNORE_UNEXPECTED_EOF)
        self.assertEqual(default, ctx.options)
        with warnings_helper.check_warnings():
            ctx.options |= ssl.OP_NO_TLSv1
        self.assertEqual(default | ssl.OP_NO_TLSv1, ctx.options)
        with warnings_helper.check_warnings():
            ctx.options = (ctx.options & ~ssl.OP_NO_TLSv1)
        self.assertEqual(default, ctx.options)
        ctx.options = 0
        # Ubuntu has OP_NO_SSLv3 forced on by default
        self.assertEqual(0, ctx.options & ~ssl.OP_NO_SSLv3)

    def test_verify_mode_protocol(self):
        with warnings_helper.check_warnings():
            ctx = ssl.SSLContext(ssl.PROTOCOL_TLS)
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

        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        self.assertEqual(ctx.verify_mode, ssl.CERT_NONE)
        self.assertFalse(ctx.check_hostname)

        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        self.assertEqual(ctx.verify_mode, ssl.CERT_REQUIRED)
        self.assertTrue(ctx.check_hostname)

    def test_hostname_checks_common_name(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        self.assertTrue(ctx.hostname_checks_common_name)
        if ssl.HAS_NEVER_CHECK_COMMON_NAME:
            ctx.hostname_checks_common_name = True
            self.assertTrue(ctx.hostname_checks_common_name)
            ctx.hostname_checks_common_name = False
            self.assertFalse(ctx.hostname_checks_common_name)
            ctx.hostname_checks_common_name = True
            self.assertTrue(ctx.hostname_checks_common_name)
        else:
            with self.assertRaises(AttributeError):
                ctx.hostname_checks_common_name = True

    @ignore_deprecation
    def test_min_max_version(self):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        # OpenSSL default is MINIMUM_SUPPORTED, however some vendors like
        # Fedora override the setting to TLS 1.0.
        minimum_range = {
            # stock OpenSSL
            ssl.TLSVersion.MINIMUM_SUPPORTED,
            # Fedora 29 uses TLS 1.0 by default
            ssl.TLSVersion.TLSv1,
            # RHEL 8 uses TLS 1.2 by default
            ssl.TLSVersion.TLSv1_2
        }