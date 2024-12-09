WRONGCERT = data_file("XXXnonexisting.pem")
BADKEY = data_file("badkey.pem")
NOKIACERT = data_file("nokia.pem")
NULLBYTECERT = data_file("nullbytecert.pem")


def handle_error(prefix):
    exc_format = ' '.join(traceback.format_exception(*sys.exc_info()))
                          ('DNS', 'projects.forum.nokia.com'))
                        )

    def test_parse_cert_CVE_2013_4073(self):
        p = ssl._ssl._test_decode_cert(NULLBYTECERT)
        if support.verbose:
            sys.stdout.write("\n" + pprint.pformat(p) + "\n")
        subject = ((('countryName', 'US'),),
                   (('stateOrProvinceName', 'Oregon'),),
                   (('localityName', 'Beaverton'),),
                   (('organizationName', 'Python Software Foundation'),),
                   (('organizationalUnitName', 'Python Core Development'),),
                   (('commonName', 'null.python.org\x00example.org'),),
                   (('emailAddress', 'python-dev@python.org'),))
        self.assertEqual(p['subject'], subject)
        self.assertEqual(p['issuer'], subject)
        self.assertEqual(p['subjectAltName'],
                         (('DNS', 'altnull.python.org\x00example.com'),
                         ('email', 'null@python.org\x00user@example.org'),
                         ('URI', 'http://null.python.org\x00http://example.org'),
                         ('IP Address', '192.0.2.1'),
                         ('IP Address', '2001:DB8:0:0:0:0:0:1\n'))
                        )

    def test_DER_to_PEM(self):
        with open(SVN_PYTHON_ORG_ROOT_CERT, 'r') as f:
            pem = f.read()
        d1 = ssl.PEM_cert_to_DER_cert(pem)
        fail(cert, 'foo.a.com')
        fail(cert, 'bar.foo.com')

        # NULL bytes are bad, CVE-2013-4073
        cert = {'subject': ((('commonName',
                              'null.python.org\x00example.org'),),)}
        ok(cert, 'null.python.org\x00example.org') # or raise an error?
        fail(cert, 'example.org')
        fail(cert, 'null.python.org')

        # Slightly fake real-world example
        cert = {'notAfter': 'Jun 26 21:41:46 2011 GMT',
                'subject': ((('commonName', 'linuxfrz.org'),),),
                'subjectAltName': (('DNS', 'linuxfr.org'),