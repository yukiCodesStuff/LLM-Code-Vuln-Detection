WRONGCERT = data_file("XXXnonexisting.pem")
BADKEY = data_file("badkey.pem")
NOKIACERT = data_file("nokia.pem")


def handle_error(prefix):
    exc_format = ' '.join(traceback.format_exception(*sys.exc_info()))
                          ('DNS', 'projects.forum.nokia.com'))
                        )

    def test_DER_to_PEM(self):
        with open(SVN_PYTHON_ORG_ROOT_CERT, 'r') as f:
            pem = f.read()
        d1 = ssl.PEM_cert_to_DER_cert(pem)
        fail(cert, 'foo.a.com')
        fail(cert, 'bar.foo.com')

        # Slightly fake real-world example
        cert = {'notAfter': 'Jun 26 21:41:46 2011 GMT',
                'subject': ((('commonName', 'linuxfrz.org'),),),
                'subjectAltName': (('DNS', 'linuxfr.org'),