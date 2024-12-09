        self.assertRaises(ValueError, ssl.match_hostname, None, 'example.com')
        self.assertRaises(ValueError, ssl.match_hostname, {}, 'example.com')

        # Issue #17980: avoid denials of service by refusing more than one
        # wildcard per fragment.
        cert = {'subject': ((('commonName', 'a*b.com'),),)}
        ok(cert, 'axxb.com')
        cert = {'subject': ((('commonName', 'a*b.co*'),),)}
        ok(cert, 'axxb.com')
        cert = {'subject': ((('commonName', 'a*b*.com'),),)}
        with self.assertRaises(ssl.CertificateError) as cm:
            ssl.match_hostname(cert, 'axxbxxc.com')
        self.assertIn("too many wildcards", str(cm.exception))

    def test_server_side(self):
        # server_hostname doesn't work for server sockets
        ctx = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
        with socket.socket() as sock: