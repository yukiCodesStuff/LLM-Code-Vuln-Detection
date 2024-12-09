            self.assertEqual(p.scheme, "http")
            self.assertEqual(p.geturl(), "http://www.python.org/javascript:alert('msg')/?query=something#fragment")

    def test_urlsplit_strip_url(self):
        noise = bytes(range(0, 0x20 + 1))
        base_url = "http://User:Pass@www.python.org:080/doc/?query=yes#frag"

        url = noise.decode("utf-8") + base_url
        p = urllib.parse.urlsplit(url)
        self.assertEqual(p.scheme, "http")
        self.assertEqual(p.netloc, "User:Pass@www.python.org:080")
        self.assertEqual(p.path, "/doc/")
        self.assertEqual(p.query, "query=yes")
        self.assertEqual(p.fragment, "frag")
        self.assertEqual(p.username, "User")
        self.assertEqual(p.password, "Pass")
        self.assertEqual(p.hostname, "www.python.org")
        self.assertEqual(p.port, 80)
        self.assertEqual(p.geturl(), base_url)

        url = noise + base_url.encode("utf-8")
        p = urllib.parse.urlsplit(url)
        self.assertEqual(p.scheme, b"http")
        self.assertEqual(p.netloc, b"User:Pass@www.python.org:080")
        self.assertEqual(p.path, b"/doc/")
        self.assertEqual(p.query, b"query=yes")
        self.assertEqual(p.fragment, b"frag")
        self.assertEqual(p.username, b"User")
        self.assertEqual(p.password, b"Pass")
        self.assertEqual(p.hostname, b"www.python.org")
        self.assertEqual(p.port, 80)
        self.assertEqual(p.geturl(), base_url.encode("utf-8"))

        # Test that trailing space is preserved as some applications rely on
        # this within query strings.
        query_spaces_url = "https://www.python.org:88/doc/?query=    "
        p = urllib.parse.urlsplit(noise.decode("utf-8") + query_spaces_url)
        self.assertEqual(p.scheme, "https")
        self.assertEqual(p.netloc, "www.python.org:88")
        self.assertEqual(p.path, "/doc/")
        self.assertEqual(p.query, "query=    ")
        self.assertEqual(p.port, 88)
        self.assertEqual(p.geturl(), query_spaces_url)

        p = urllib.parse.urlsplit("www.pypi.org ")
        # That "hostname" gets considered a "path" due to the
        # trailing space and our existing logic...  YUCK...
        # and re-assembles via geturl aka unurlsplit into the original.
        # django.core.validators.URLValidator (at least through v3.2) relies on
        # this, for better or worse, to catch it in a ValidationError via its
        # regular expressions.
        # Here we test the basic round trip concept of such a trailing space.
        self.assertEqual(urllib.parse.urlunsplit(p), "www.pypi.org ")

        # with scheme as cache-key
        url = "//www.python.org/"
        scheme = noise.decode("utf-8") + "https" + noise.decode("utf-8")
        for _ in range(2):
            p = urllib.parse.urlsplit(url, scheme=scheme)
            self.assertEqual(p.scheme, "https")
            self.assertEqual(p.geturl(), "https://www.python.org/")

    def test_attributes_bad_port(self):
        """Check handling of invalid ports."""
        for bytes in (False, True):
            for parse in (urllib.parse.urlsplit, urllib.parse.urlparse):
                for port in ("foo", "1.5", "-1", "0x10", "-0", "1_1", " 1", "1 ", "६"):
                    with self.subTest(bytes=bytes, parse=parse, port=port):
                        netloc = "www.example.net:" + port
                        url = "http://" + netloc + "/"
                        if bytes:
                            if netloc.isascii() and port.isascii():
                                netloc = netloc.encode("ascii")
                                url = url.encode("ascii")