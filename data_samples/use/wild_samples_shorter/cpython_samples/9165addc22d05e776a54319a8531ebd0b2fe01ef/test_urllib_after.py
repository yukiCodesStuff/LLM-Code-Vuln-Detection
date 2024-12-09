            self.unfakehttp()

    @unittest.skipUnless(ssl, "ssl module required")
    def test_url_path_with_control_char_rejected(self):
        for char_no in list(range(0, 0x21)) + [0x7f]:
            char = chr(char_no)
            schemeless_url = f"//localhost:7777/test{char}/"
            self.fakehttp(b"HTTP/1.1 200 OK\r\n\r\nHello.")
                self.unfakehttp()

    @unittest.skipUnless(ssl, "ssl module required")
    def test_url_path_with_newline_header_injection_rejected(self):
        self.fakehttp(b"HTTP/1.1 200 OK\r\n\r\nHello.")
        host = "localhost:7777?a=1 HTTP/1.1\r\nX-injected: header\r\nTEST: 123"
        schemeless_url = "//" + host + ":8080/test/?test=a"
        try:
        finally:
            self.unfakehttp()

    @unittest.skipUnless(ssl, "ssl module required")
    def test_url_host_with_control_char_rejected(self):
        for char_no in list(range(0, 0x21)) + [0x7f]:
            char = chr(char_no)
            schemeless_url = f"//localhost{char}/test/"
            self.fakehttp(b"HTTP/1.1 200 OK\r\n\r\nHello.")
            try:
                escaped_char_repr = repr(char).replace('\\', r'\\')
                InvalidURL = http.client.InvalidURL
                with self.assertRaisesRegex(
                    InvalidURL, f"contain control.*{escaped_char_repr}"):
                    urlopen(f"http:{schemeless_url}")
                with self.assertRaisesRegex(InvalidURL, f"contain control.*{escaped_char_repr}"):
                    urlopen(f"https:{schemeless_url}")
            finally:
                self.unfakehttp()

    @unittest.skipUnless(ssl, "ssl module required")
    def test_url_host_with_newline_header_injection_rejected(self):
        self.fakehttp(b"HTTP/1.1 200 OK\r\n\r\nHello.")
        host = "localhost\r\nX-injected: header\r\n"
        schemeless_url = "//" + host + ":8080/test/?test=a"
        try:
            InvalidURL = http.client.InvalidURL
            with self.assertRaisesRegex(
                InvalidURL, r"contain control.*\\r"):
                urlopen(f"http:{schemeless_url}")
            with self.assertRaisesRegex(InvalidURL, r"contain control.*\\n"):
                urlopen(f"https:{schemeless_url}")
        finally:
            self.unfakehttp()

    def test_read_0_9(self):
        # "0.9" response accepted (but not "simple responses" without
        # a status line)
        self.check_read(b"0.9")