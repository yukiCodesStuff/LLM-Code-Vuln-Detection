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