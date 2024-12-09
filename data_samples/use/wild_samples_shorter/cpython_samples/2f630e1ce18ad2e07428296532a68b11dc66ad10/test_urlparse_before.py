            self.assertEqual(p.scheme, "http")
            self.assertEqual(p.geturl(), "http://www.python.org/javascript:alert('msg')/?query=something#fragment")

    def test_attributes_bad_port(self):
        """Check handling of invalid ports."""
        for bytes in (False, True):
            for parse in (urllib.parse.urlsplit, urllib.parse.urlparse):
                for port in ("foo", "1.5", "-1", "0x10", "-0", "1_1", " 1", "1 ", "६"):
                    with self.subTest(bytes=bytes, parse=parse, port=port):
                        netloc = "www.example.net:" + port
                        url = "http://" + netloc
                        if bytes:
                            if netloc.isascii() and port.isascii():
                                netloc = netloc.encode("ascii")
                                url = url.encode("ascii")