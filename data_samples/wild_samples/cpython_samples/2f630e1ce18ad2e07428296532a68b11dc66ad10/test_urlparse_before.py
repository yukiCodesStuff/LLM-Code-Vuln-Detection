        for _ in range(2):
            p = urllib.parse.urlsplit(url, scheme=scheme)
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
                            else:
                                continue
                        p = parse(url)
                        self.assertEqual(p.netloc, netloc)
                        with self.assertRaises(ValueError):
                            p.port

    def test_attributes_bad_scheme(self):
        """Check handling of invalid schemes."""
        for bytes in (False, True):
            for parse in (urllib.parse.urlsplit, urllib.parse.urlparse):
                for scheme in (".", "+", "-", "0", "http&", "६http"):
                    with self.subTest(bytes=bytes, parse=parse, scheme=scheme):
                        url = scheme + "://www.example.net"
                        if bytes:
                            if url.isascii():
                                url = url.encode("ascii")
                            else:
                                continue
                        p = parse(url)
                        if bytes:
                            self.assertEqual(p.scheme, b"")
                        else:
                            self.assertEqual(p.scheme, "")

    def test_attributes_without_netloc(self):
        # This example is straight from RFC 3261.  It looks like it
        # should allow the username, hostname, and port to be filled
        # in, but doesn't.  Since it's a URI and doesn't use the
        # scheme://netloc syntax, the netloc and related attributes
        # should be left empty.
        uri = "sip:alice@atlanta.com;maddr=239.255.255.1;ttl=15"
        p = urllib.parse.urlsplit(uri)
        self.assertEqual(p.netloc, "")
        self.assertEqual(p.username, None)
        self.assertEqual(p.password, None)
        self.assertEqual(p.hostname, None)
        self.assertEqual(p.port, None)
        self.assertEqual(p.geturl(), uri)

        p = urllib.parse.urlparse(uri)
        self.assertEqual(p.netloc, "")
        self.assertEqual(p.username, None)
        self.assertEqual(p.password, None)
        self.assertEqual(p.hostname, None)
        self.assertEqual(p.port, None)
        self.assertEqual(p.geturl(), uri)

        # You guessed it, repeating the test with bytes input
        uri = b"sip:alice@atlanta.com;maddr=239.255.255.1;ttl=15"
        p = urllib.parse.urlsplit(uri)
        self.assertEqual(p.netloc, b"")
        self.assertEqual(p.username, None)
        self.assertEqual(p.password, None)
        self.assertEqual(p.hostname, None)
        self.assertEqual(p.port, None)
        self.assertEqual(p.geturl(), uri)

        p = urllib.parse.urlparse(uri)
        self.assertEqual(p.netloc, b"")
        self.assertEqual(p.username, None)
        self.assertEqual(p.password, None)
        self.assertEqual(p.hostname, None)
        self.assertEqual(p.port, None)
        self.assertEqual(p.geturl(), uri)

    def test_noslash(self):
        # Issue 1637: http://foo.com?query is legal
        self.assertEqual(urllib.parse.urlparse("http://example.com?blahblah=/foo"),
                         ('http', 'example.com', '', '', 'blahblah=/foo', ''))
        self.assertEqual(urllib.parse.urlparse(b"http://example.com?blahblah=/foo"),
                         (b'http', b'example.com', b'', b'', b'blahblah=/foo', b''))

    def test_withoutscheme(self):
        # Test urlparse without scheme
        # Issue 754016: urlparse goes wrong with IP:port without scheme
        # RFC 1808 specifies that netloc should start with //, urlparse expects
        # the same, otherwise it classifies the portion of url as path.
        self.assertEqual(urllib.parse.urlparse("path"),
                ('','','path','','',''))
        self.assertEqual(urllib.parse.urlparse("//www.python.org:80"),
                ('','www.python.org:80','','','',''))
        self.assertEqual(urllib.parse.urlparse("http://www.python.org:80"),
                ('http','www.python.org:80','','','',''))
        # Repeat for bytes input
        self.assertEqual(urllib.parse.urlparse(b"path"),
                (b'',b'',b'path',b'',b'',b''))
        self.assertEqual(urllib.parse.urlparse(b"//www.python.org:80"),
                (b'',b'www.python.org:80',b'',b'',b'',b''))
        self.assertEqual(urllib.parse.urlparse(b"http://www.python.org:80"),
                (b'http',b'www.python.org:80',b'',b'',b'',b''))

    def test_portseparator(self):
        # Issue 754016 makes changes for port separator ':' from scheme separator
        self.assertEqual(urllib.parse.urlparse("http:80"), ('http','','80','','',''))
        self.assertEqual(urllib.parse.urlparse("https:80"), ('https','','80','','',''))
        self.assertEqual(urllib.parse.urlparse("path:80"), ('path','','80','','',''))
        self.assertEqual(urllib.parse.urlparse("http:"),('http','','','','',''))
        self.assertEqual(urllib.parse.urlparse("https:"),('https','','','','',''))
        self.assertEqual(urllib.parse.urlparse("http://www.python.org:80"),
                ('http','www.python.org:80','','','',''))
        # As usual, need to check bytes input as well
        self.assertEqual(urllib.parse.urlparse(b"http:80"), (b'http',b'',b'80',b'',b'',b''))
        self.assertEqual(urllib.parse.urlparse(b"https:80"), (b'https',b'',b'80',b'',b'',b''))
        self.assertEqual(urllib.parse.urlparse(b"path:80"), (b'path',b'',b'80',b'',b'',b''))
        self.assertEqual(urllib.parse.urlparse(b"http:"),(b'http',b'',b'',b'',b'',b''))
        self.assertEqual(urllib.parse.urlparse(b"https:"),(b'https',b'',b'',b'',b'',b''))
        self.assertEqual(urllib.parse.urlparse(b"http://www.python.org:80"),
                (b'http',b'www.python.org:80',b'',b'',b'',b''))

    def test_usingsys(self):
        # Issue 3314: sys module is used in the error
        self.assertRaises(TypeError, urllib.parse.urlencode, "foo")

    def test_anyscheme(self):
        # Issue 7904: s3://foo.com/stuff has netloc "foo.com".
        self.assertEqual(urllib.parse.urlparse("s3://foo.com/stuff"),
                         ('s3', 'foo.com', '/stuff', '', '', ''))
        self.assertEqual(urllib.parse.urlparse("x-newscheme://foo.com/stuff"),
                         ('x-newscheme', 'foo.com', '/stuff', '', '', ''))
        self.assertEqual(urllib.parse.urlparse("x-newscheme://foo.com/stuff?query#fragment"),
                         ('x-newscheme', 'foo.com', '/stuff', '', 'query', 'fragment'))
        self.assertEqual(urllib.parse.urlparse("x-newscheme://foo.com/stuff?query"),
                         ('x-newscheme', 'foo.com', '/stuff', '', 'query', ''))

        # And for bytes...
        self.assertEqual(urllib.parse.urlparse(b"s3://foo.com/stuff"),
                         (b's3', b'foo.com', b'/stuff', b'', b'', b''))
        self.assertEqual(urllib.parse.urlparse(b"x-newscheme://foo.com/stuff"),
                         (b'x-newscheme', b'foo.com', b'/stuff', b'', b'', b''))
        self.assertEqual(urllib.parse.urlparse(b"x-newscheme://foo.com/stuff?query#fragment"),
                         (b'x-newscheme', b'foo.com', b'/stuff', b'', b'query', b'fragment'))
        self.assertEqual(urllib.parse.urlparse(b"x-newscheme://foo.com/stuff?query"),
                         (b'x-newscheme', b'foo.com', b'/stuff', b'', b'query', b''))

    def test_default_scheme(self):
        # Exercise the scheme parameter of urlparse() and urlsplit()
        for func in (urllib.parse.urlparse, urllib.parse.urlsplit):
            with self.subTest(function=func):
                result = func("http://example.net/", "ftp")
                self.assertEqual(result.scheme, "http")
                result = func(b"http://example.net/", b"ftp")
                self.assertEqual(result.scheme, b"http")
                self.assertEqual(func("path", "ftp").scheme, "ftp")
                self.assertEqual(func("path", scheme="ftp").scheme, "ftp")
                self.assertEqual(func(b"path", scheme=b"ftp").scheme, b"ftp")
                self.assertEqual(func("path").scheme, "")
                self.assertEqual(func(b"path").scheme, b"")
                self.assertEqual(func(b"path", "").scheme, b"")

    def test_parse_fragments(self):
        # Exercise the allow_fragments parameter of urlparse() and urlsplit()
        tests = (
            ("http:#frag", "path", "frag"),
            ("//example.net#frag", "path", "frag"),
            ("index.html#frag", "path", "frag"),
            (";a=b#frag", "params", "frag"),
            ("?a=b#frag", "query", "frag"),
            ("#frag", "path", "frag"),
            ("abc#@frag", "path", "@frag"),
            ("//abc#@frag", "path", "@frag"),
            ("//abc:80#@frag", "path", "@frag"),
            ("//abc#@frag:80", "path", "@frag:80"),
        )
        for url, attr, expected_frag in tests:
            for func in (urllib.parse.urlparse, urllib.parse.urlsplit):
                if attr == "params" and func is urllib.parse.urlsplit:
                    attr = "path"
                with self.subTest(url=url, function=func):
                    result = func(url, allow_fragments=False)
                    self.assertEqual(result.fragment, "")
                    self.assertTrue(
                            getattr(result, attr).endswith("#" + expected_frag))
                    self.assertEqual(func(url, "", False).fragment, "")

                    result = func(url, allow_fragments=True)
                    self.assertEqual(result.fragment, expected_frag)
                    self.assertFalse(
                            getattr(result, attr).endswith(expected_frag))
                    self.assertEqual(func(url, "", True).fragment,
                                     expected_frag)
                    self.assertEqual(func(url).fragment, expected_frag)

    def test_mixed_types_rejected(self):
        # Several functions that process either strings or ASCII encoded bytes
        # accept multiple arguments. Check they reject mixed type input
        with self.assertRaisesRegex(TypeError, "Cannot mix str"):
            urllib.parse.urlparse("www.python.org", b"http")
        with self.assertRaisesRegex(TypeError, "Cannot mix str"):
            urllib.parse.urlparse(b"www.python.org", "http")
        with self.assertRaisesRegex(TypeError, "Cannot mix str"):
            urllib.parse.urlsplit("www.python.org", b"http")
        with self.assertRaisesRegex(TypeError, "Cannot mix str"):
            urllib.parse.urlsplit(b"www.python.org", "http")
        with self.assertRaisesRegex(TypeError, "Cannot mix str"):
            urllib.parse.urlunparse(( b"http", "www.python.org","","","",""))
        with self.assertRaisesRegex(TypeError, "Cannot mix str"):
            urllib.parse.urlunparse(("http", b"www.python.org","","","",""))
        with self.assertRaisesRegex(TypeError, "Cannot mix str"):
            urllib.parse.urlunsplit((b"http", "www.python.org","","",""))
        with self.assertRaisesRegex(TypeError, "Cannot mix str"):
            urllib.parse.urlunsplit(("http", b"www.python.org","","",""))
        with self.assertRaisesRegex(TypeError, "Cannot mix str"):
            urllib.parse.urljoin("http://python.org", b"http://python.org")
        with self.assertRaisesRegex(TypeError, "Cannot mix str"):
            urllib.parse.urljoin(b"http://python.org", "http://python.org")

    def _check_result_type(self, str_type):
        num_args = len(str_type._fields)
        bytes_type = str_type._encoded_counterpart
        self.assertIs(bytes_type._decoded_counterpart, str_type)
        str_args = ('',) * num_args
        bytes_args = (b'',) * num_args
        str_result = str_type(*str_args)
        bytes_result = bytes_type(*bytes_args)
        encoding = 'ascii'
        errors = 'strict'
        self.assertEqual(str_result, str_args)
        self.assertEqual(bytes_result.decode(), str_args)
        self.assertEqual(bytes_result.decode(), str_result)
        self.assertEqual(bytes_result.decode(encoding), str_args)
        self.assertEqual(bytes_result.decode(encoding), str_result)
        self.assertEqual(bytes_result.decode(encoding, errors), str_args)
        self.assertEqual(bytes_result.decode(encoding, errors), str_result)
        self.assertEqual(bytes_result, bytes_args)
        self.assertEqual(str_result.encode(), bytes_args)
        self.assertEqual(str_result.encode(), bytes_result)
        self.assertEqual(str_result.encode(encoding), bytes_args)
        self.assertEqual(str_result.encode(encoding), bytes_result)
        self.assertEqual(str_result.encode(encoding, errors), bytes_args)
        self.assertEqual(str_result.encode(encoding, errors), bytes_result)

    def test_result_pairs(self):
        # Check encoding and decoding between result pairs
        result_types = [
          urllib.parse.DefragResult,
          urllib.parse.SplitResult,
          urllib.parse.ParseResult,
        ]
        for result_type in result_types:
            self._check_result_type(result_type)

    def test_parse_qs_encoding(self):
        result = urllib.parse.parse_qs("key=\u0141%E9", encoding="latin-1")
        self.assertEqual(result, {'key': ['\u0141\xE9']})
        result = urllib.parse.parse_qs("key=\u0141%C3%A9", encoding="utf-8")
        self.assertEqual(result, {'key': ['\u0141\xE9']})
        result = urllib.parse.parse_qs("key=\u0141%C3%A9", encoding="ascii")
        self.assertEqual(result, {'key': ['\u0141\ufffd\ufffd']})
        result = urllib.parse.parse_qs("key=\u0141%E9-", encoding="ascii")
        self.assertEqual(result, {'key': ['\u0141\ufffd-']})
        result = urllib.parse.parse_qs("key=\u0141%E9-", encoding="ascii",
                                                          errors="ignore")
        self.assertEqual(result, {'key': ['\u0141-']})

    def test_parse_qsl_encoding(self):
        result = urllib.parse.parse_qsl("key=\u0141%E9", encoding="latin-1")
        self.assertEqual(result, [('key', '\u0141\xE9')])
        result = urllib.parse.parse_qsl("key=\u0141%C3%A9", encoding="utf-8")
        self.assertEqual(result, [('key', '\u0141\xE9')])
        result = urllib.parse.parse_qsl("key=\u0141%C3%A9", encoding="ascii")
        self.assertEqual(result, [('key', '\u0141\ufffd\ufffd')])
        result = urllib.parse.parse_qsl("key=\u0141%E9-", encoding="ascii")
        self.assertEqual(result, [('key', '\u0141\ufffd-')])
        result = urllib.parse.parse_qsl("key=\u0141%E9-", encoding="ascii",
                                                          errors="ignore")
        self.assertEqual(result, [('key', '\u0141-')])

    def test_parse_qsl_max_num_fields(self):
        with self.assertRaises(ValueError):
            urllib.parse.parse_qs('&'.join(['a=a']*11), max_num_fields=10)
        urllib.parse.parse_qs('&'.join(['a=a']*10), max_num_fields=10)

    def test_parse_qs_separator(self):
        parse_qs_semicolon_cases = [
            (";", {}),
            (";;", {}),
            (";a=b", {'a': ['b']}),
            ("a=a+b;b=b+c", {'a': ['a b'], 'b': ['b c']}),
            ("a=1;a=2", {'a': ['1', '2']}),
            (b";", {}),
            (b";;", {}),
            (b";a=b", {b'a': [b'b']}),
            (b"a=a+b;b=b+c", {b'a': [b'a b'], b'b': [b'b c']}),
            (b"a=1;a=2", {b'a': [b'1', b'2']}),
        ]
        for orig, expect in parse_qs_semicolon_cases:
            with self.subTest(f"Original: {orig!r}, Expected: {expect!r}"):
                result = urllib.parse.parse_qs(orig, separator=';')
                self.assertEqual(result, expect, "Error parsing %r" % orig)
                result_bytes = urllib.parse.parse_qs(orig, separator=b';')
                self.assertEqual(result_bytes, expect, "Error parsing %r" % orig)


    def test_parse_qsl_separator(self):
        parse_qsl_semicolon_cases = [
            (";", []),
            (";;", []),
            (";a=b", [('a', 'b')]),
            ("a=a+b;b=b+c", [('a', 'a b'), ('b', 'b c')]),
            ("a=1;a=2", [('a', '1'), ('a', '2')]),
            (b";", []),
            (b";;", []),
            (b";a=b", [(b'a', b'b')]),
            (b"a=a+b;b=b+c", [(b'a', b'a b'), (b'b', b'b c')]),
            (b"a=1;a=2", [(b'a', b'1'), (b'a', b'2')]),
        ]
        for orig, expect in parse_qsl_semicolon_cases:
            with self.subTest(f"Original: {orig!r}, Expected: {expect!r}"):
                result = urllib.parse.parse_qsl(orig, separator=';')
                self.assertEqual(result, expect, "Error parsing %r" % orig)
                result_bytes = urllib.parse.parse_qsl(orig, separator=b';')
                self.assertEqual(result_bytes, expect, "Error parsing %r" % orig)


    def test_urlencode_sequences(self):
        # Other tests incidentally urlencode things; test non-covered cases:
        # Sequence and object values.
        result = urllib.parse.urlencode({'a': [1, 2], 'b': (3, 4, 5)}, True)
        # we cannot rely on ordering here
        assert set(result.split('&')) == {'a=1', 'a=2', 'b=3', 'b=4', 'b=5'}

        class Trivial:
            def __str__(self):
                return 'trivial'

        result = urllib.parse.urlencode({'a': Trivial()}, True)
        self.assertEqual(result, 'a=trivial')

    def test_urlencode_quote_via(self):
        result = urllib.parse.urlencode({'a': 'some value'})
        self.assertEqual(result, "a=some+value")
        result = urllib.parse.urlencode({'a': 'some value/another'},
                                        quote_via=urllib.parse.quote)
        self.assertEqual(result, "a=some%20value%2Fanother")
        result = urllib.parse.urlencode({'a': 'some value/another'},
                                        safe='/', quote_via=urllib.parse.quote)
        self.assertEqual(result, "a=some%20value/another")

    def test_quote_from_bytes(self):
        self.assertRaises(TypeError, urllib.parse.quote_from_bytes, 'foo')
        result = urllib.parse.quote_from_bytes(b'archaeological arcana')
        self.assertEqual(result, 'archaeological%20arcana')
        result = urllib.parse.quote_from_bytes(b'')
        self.assertEqual(result, '')
        result = urllib.parse.quote_from_bytes(b'A'*10_000)
        self.assertEqual(result, 'A'*10_000)
        result = urllib.parse.quote_from_bytes(b'z\x01/ '*253_183)
        self.assertEqual(result, 'z%01/%20'*253_183)

    def test_unquote_to_bytes(self):
        result = urllib.parse.unquote_to_bytes('abc%20def')
        self.assertEqual(result, b'abc def')
        result = urllib.parse.unquote_to_bytes('')
        self.assertEqual(result, b'')

    def test_quote_errors(self):
        self.assertRaises(TypeError, urllib.parse.quote, b'foo',
                          encoding='utf-8')
        self.assertRaises(TypeError, urllib.parse.quote, b'foo', errors='strict')

    def test_issue14072(self):
        p1 = urllib.parse.urlsplit('tel:+31-641044153')
        self.assertEqual(p1.scheme, 'tel')
        self.assertEqual(p1.path, '+31-641044153')
        p2 = urllib.parse.urlsplit('tel:+31641044153')
        self.assertEqual(p2.scheme, 'tel')
        self.assertEqual(p2.path, '+31641044153')
        # assert the behavior for urlparse
        p1 = urllib.parse.urlparse('tel:+31-641044153')
        self.assertEqual(p1.scheme, 'tel')
        self.assertEqual(p1.path, '+31-641044153')
        p2 = urllib.parse.urlparse('tel:+31641044153')
        self.assertEqual(p2.scheme, 'tel')
        self.assertEqual(p2.path, '+31641044153')

    def test_invalid_bracketed_hosts(self):
        self.assertRaises(ValueError, urllib.parse.urlsplit, 'Scheme://user@[192.0.2.146]/Path?Query')
        self.assertRaises(ValueError, urllib.parse.urlsplit, 'Scheme://user@[important.com:8000]/Path?Query')
        self.assertRaises(ValueError, urllib.parse.urlsplit, 'Scheme://user@[v123r.IP]/Path?Query')
        self.assertRaises(ValueError, urllib.parse.urlsplit, 'Scheme://user@[v12ae]/Path?Query')
        self.assertRaises(ValueError, urllib.parse.urlsplit, 'Scheme://user@[v.IP]/Path?Query')
        self.assertRaises(ValueError, urllib.parse.urlsplit, 'Scheme://user@[v123.]/Path?Query')
        self.assertRaises(ValueError, urllib.parse.urlsplit, 'Scheme://user@[v]/Path?Query')
        self.assertRaises(ValueError, urllib.parse.urlsplit, 'Scheme://user@[0439:23af::2309::fae7:1234]/Path?Query')
        self.assertRaises(ValueError, urllib.parse.urlsplit, 'Scheme://user@[0439:23af:2309::fae7:1234:2342:438e:192.0.2.146]/Path?Query')
        self.assertRaises(ValueError, urllib.parse.urlsplit, 'Scheme://user@]v6a.ip[/Path')

    def test_splitting_bracketed_hosts(self):
        p1 = urllib.parse.urlsplit('scheme://user@[v6a.ip]/path?query')
        self.assertEqual(p1.hostname, 'v6a.ip')
        self.assertEqual(p1.username, 'user')
        self.assertEqual(p1.path, '/path')
        p2 = urllib.parse.urlsplit('scheme://user@[0439:23af:2309::fae7%test]/path?query')
        self.assertEqual(p2.hostname, '0439:23af:2309::fae7%test')
        self.assertEqual(p2.username, 'user')
        self.assertEqual(p2.path, '/path')
        p3 = urllib.parse.urlsplit('scheme://user@[0439:23af:2309::fae7:1234:192.0.2.146%test]/path?query')
        self.assertEqual(p3.hostname, '0439:23af:2309::fae7:1234:192.0.2.146%test')
        self.assertEqual(p3.username, 'user')
        self.assertEqual(p3.path, '/path')

    def test_port_casting_failure_message(self):
        message = "Port could not be cast to integer value as 'oracle'"
        p1 = urllib.parse.urlparse('http://Server=sde; Service=sde:oracle')
        with self.assertRaisesRegex(ValueError, message):
            p1.port

        p2 = urllib.parse.urlsplit('http://Server=sde; Service=sde:oracle')
        with self.assertRaisesRegex(ValueError, message):
            p2.port

    def test_telurl_params(self):
        p1 = urllib.parse.urlparse('tel:123-4;phone-context=+1-650-516')
        self.assertEqual(p1.scheme, 'tel')
        self.assertEqual(p1.path, '123-4')
        self.assertEqual(p1.params, 'phone-context=+1-650-516')

        p1 = urllib.parse.urlparse('tel:+1-201-555-0123')
        self.assertEqual(p1.scheme, 'tel')
        self.assertEqual(p1.path, '+1-201-555-0123')
        self.assertEqual(p1.params, '')

        p1 = urllib.parse.urlparse('tel:7042;phone-context=example.com')
        self.assertEqual(p1.scheme, 'tel')
        self.assertEqual(p1.path, '7042')
        self.assertEqual(p1.params, 'phone-context=example.com')

        p1 = urllib.parse.urlparse('tel:863-1234;phone-context=+1-914-555')
        self.assertEqual(p1.scheme, 'tel')
        self.assertEqual(p1.path, '863-1234')
        self.assertEqual(p1.params, 'phone-context=+1-914-555')

    def test_Quoter_repr(self):
        quoter = urllib.parse._Quoter(urllib.parse._ALWAYS_SAFE)
        self.assertIn('Quoter', repr(quoter))

    def test_clear_cache_for_code_coverage(self):
        urllib.parse.clear_cache()

    def test_urllib_parse_getattr_failure(self):
        """Test that urllib.parse.__getattr__() fails correctly."""
        with self.assertRaises(AttributeError):
            unused = urllib.parse.this_does_not_exist

    def test_all(self):
        expected = []
        undocumented = {
            'splitattr', 'splithost', 'splitnport', 'splitpasswd',
            'splitport', 'splitquery', 'splittag', 'splittype', 'splituser',
            'splitvalue',
            'ResultBase', 'clear_cache', 'to_bytes', 'unwrap',
        }
        for name in dir(urllib.parse):
            if name.startswith('_') or name in undocumented:
                continue
            object = getattr(urllib.parse, name)
            if getattr(object, '__module__', None) == 'urllib.parse':
                expected.append(name)
        self.assertCountEqual(urllib.parse.__all__, expected)

    def test_urlsplit_normalization(self):
        # Certain characters should never occur in the netloc,
        # including under normalization.
        # Ensure that ALL of them are detected and cause an error
        illegal_chars = '/:#?@'
        hex_chars = {'{:04X}'.format(ord(c)) for c in illegal_chars}
        denorm_chars = [
            c for c in map(chr, range(128, sys.maxunicode))
            if unicodedata.decomposition(c)
            and (hex_chars & set(unicodedata.decomposition(c).split()))
            and c not in illegal_chars
        ]
        # Sanity check that we found at least one such character
        self.assertIn('\u2100', denorm_chars)
        self.assertIn('\uFF03', denorm_chars)

        # bpo-36742: Verify port separators are ignored when they
        # existed prior to decomposition
        urllib.parse.urlsplit('http://\u30d5\u309a:80')
        with self.assertRaises(ValueError):
            urllib.parse.urlsplit('http://\u30d5\u309a\ufe1380')

        for scheme in ["http", "https", "ftp"]:
            for netloc in ["netloc{}false.netloc", "n{}user@netloc"]:
                for c in denorm_chars:
                    url = "{}://{}/path".format(scheme, netloc.format(c))
                    with self.subTest(url=url, char='{:04X}'.format(ord(c))):
                        with self.assertRaises(ValueError):
                            urllib.parse.urlsplit(url)

class Utility_Tests(unittest.TestCase):
    """Testcase to test the various utility functions in the urllib."""
    # In Python 2 this test class was in test_urllib.

    def test_splittype(self):
        splittype = urllib.parse._splittype
        self.assertEqual(splittype('type:opaquestring'), ('type', 'opaquestring'))
        self.assertEqual(splittype('opaquestring'), (None, 'opaquestring'))
        self.assertEqual(splittype(':opaquestring'), (None, ':opaquestring'))
        self.assertEqual(splittype('type:'), ('type', ''))
        self.assertEqual(splittype('type:opaque:string'), ('type', 'opaque:string'))

    def test_splithost(self):
        splithost = urllib.parse._splithost
        self.assertEqual(splithost('//www.example.org:80/foo/bar/baz.html'),
                         ('www.example.org:80', '/foo/bar/baz.html'))
        self.assertEqual(splithost('//www.example.org:80'),
                         ('www.example.org:80', ''))
        self.assertEqual(splithost('/foo/bar/baz.html'),
                         (None, '/foo/bar/baz.html'))

        # bpo-30500: # starts a fragment.
        self.assertEqual(splithost('//127.0.0.1#@host.com'),
                         ('127.0.0.1', '/#@host.com'))
        self.assertEqual(splithost('//127.0.0.1#@host.com:80'),
                         ('127.0.0.1', '/#@host.com:80'))
        self.assertEqual(splithost('//127.0.0.1:80#@host.com'),
                         ('127.0.0.1:80', '/#@host.com'))

        # Empty host is returned as empty string.
        self.assertEqual(splithost("///file"),
                         ('', '/file'))

        # Trailing semicolon, question mark and hash symbol are kept.
        self.assertEqual(splithost("//example.net/file;"),
                         ('example.net', '/file;'))
        self.assertEqual(splithost("//example.net/file?"),
                         ('example.net', '/file?'))
        self.assertEqual(splithost("//example.net/file#"),
                         ('example.net', '/file#'))

    def test_splituser(self):
        splituser = urllib.parse._splituser
        self.assertEqual(splituser('User:Pass@www.python.org:080'),
                         ('User:Pass', 'www.python.org:080'))
        self.assertEqual(splituser('@www.python.org:080'),
                         ('', 'www.python.org:080'))
        self.assertEqual(splituser('www.python.org:080'),
                         (None, 'www.python.org:080'))
        self.assertEqual(splituser('User:Pass@'),
                         ('User:Pass', ''))
        self.assertEqual(splituser('User@example.com:Pass@www.python.org:080'),
                         ('User@example.com:Pass', 'www.python.org:080'))

    def test_splitpasswd(self):
        # Some of the password examples are not sensible, but it is added to
        # confirming to RFC2617 and addressing issue4675.
        splitpasswd = urllib.parse._splitpasswd
        self.assertEqual(splitpasswd('user:ab'), ('user', 'ab'))
        self.assertEqual(splitpasswd('user:a\nb'), ('user', 'a\nb'))
        self.assertEqual(splitpasswd('user:a\tb'), ('user', 'a\tb'))
        self.assertEqual(splitpasswd('user:a\rb'), ('user', 'a\rb'))
        self.assertEqual(splitpasswd('user:a\fb'), ('user', 'a\fb'))
        self.assertEqual(splitpasswd('user:a\vb'), ('user', 'a\vb'))
        self.assertEqual(splitpasswd('user:a:b'), ('user', 'a:b'))
        self.assertEqual(splitpasswd('user:a b'), ('user', 'a b'))
        self.assertEqual(splitpasswd('user 2:ab'), ('user 2', 'ab'))
        self.assertEqual(splitpasswd('user+1:a+b'), ('user+1', 'a+b'))
        self.assertEqual(splitpasswd('user:'), ('user', ''))
        self.assertEqual(splitpasswd('user'), ('user', None))
        self.assertEqual(splitpasswd(':ab'), ('', 'ab'))

    def test_splitport(self):
        splitport = urllib.parse._splitport
        self.assertEqual(splitport('parrot:88'), ('parrot', '88'))
        self.assertEqual(splitport('parrot'), ('parrot', None))
        self.assertEqual(splitport('parrot:'), ('parrot', None))
        self.assertEqual(splitport('127.0.0.1'), ('127.0.0.1', None))
        self.assertEqual(splitport('parrot:cheese'), ('parrot:cheese', None))
        self.assertEqual(splitport('[::1]:88'), ('[::1]', '88'))
        self.assertEqual(splitport('[::1]'), ('[::1]', None))
        self.assertEqual(splitport(':88'), ('', '88'))

    def test_splitnport(self):
        splitnport = urllib.parse._splitnport
        self.assertEqual(splitnport('parrot:88'), ('parrot', 88))
        self.assertEqual(splitnport('parrot'), ('parrot', -1))
        self.assertEqual(splitnport('parrot', 55), ('parrot', 55))
        self.assertEqual(splitnport('parrot:'), ('parrot', -1))
        self.assertEqual(splitnport('parrot:', 55), ('parrot', 55))
        self.assertEqual(splitnport('127.0.0.1'), ('127.0.0.1', -1))
        self.assertEqual(splitnport('127.0.0.1', 55), ('127.0.0.1', 55))
        self.assertEqual(splitnport('parrot:cheese'), ('parrot', None))
        self.assertEqual(splitnport('parrot:cheese', 55), ('parrot', None))
        self.assertEqual(splitnport('parrot: +1_0 '), ('parrot', None))

    def test_splitquery(self):
        # Normal cases are exercised by other tests; ensure that we also
        # catch cases with no port specified (testcase ensuring coverage)
        splitquery = urllib.parse._splitquery
        self.assertEqual(splitquery('http://python.org/fake?foo=bar'),
                         ('http://python.org/fake', 'foo=bar'))
        self.assertEqual(splitquery('http://python.org/fake?foo=bar?'),
                         ('http://python.org/fake?foo=bar', ''))
        self.assertEqual(splitquery('http://python.org/fake'),
                         ('http://python.org/fake', None))
        self.assertEqual(splitquery('?foo=bar'), ('', 'foo=bar'))

    def test_splittag(self):
        splittag = urllib.parse._splittag
        self.assertEqual(splittag('http://example.com?foo=bar#baz'),
                         ('http://example.com?foo=bar', 'baz'))
        self.assertEqual(splittag('http://example.com?foo=bar#'),
                         ('http://example.com?foo=bar', ''))
        self.assertEqual(splittag('#baz'), ('', 'baz'))
        self.assertEqual(splittag('http://example.com?foo=bar'),
                         ('http://example.com?foo=bar', None))
        self.assertEqual(splittag('http://example.com?foo=bar#baz#boo'),
                         ('http://example.com?foo=bar#baz', 'boo'))

    def test_splitattr(self):
        splitattr = urllib.parse._splitattr
        self.assertEqual(splitattr('/path;attr1=value1;attr2=value2'),
                         ('/path', ['attr1=value1', 'attr2=value2']))
        self.assertEqual(splitattr('/path;'), ('/path', ['']))
        self.assertEqual(splitattr(';attr1=value1;attr2=value2'),
                         ('', ['attr1=value1', 'attr2=value2']))
        self.assertEqual(splitattr('/path'), ('/path', []))

    def test_splitvalue(self):
        # Normal cases are exercised by other tests; test pathological cases
        # with no key/value pairs. (testcase ensuring coverage)
        splitvalue = urllib.parse._splitvalue
        self.assertEqual(splitvalue('foo=bar'), ('foo', 'bar'))
        self.assertEqual(splitvalue('foo='), ('foo', ''))
        self.assertEqual(splitvalue('=bar'), ('', 'bar'))
        self.assertEqual(splitvalue('foobar'), ('foobar', None))
        self.assertEqual(splitvalue('foo=bar=baz'), ('foo', 'bar=baz'))

    def test_to_bytes(self):
        result = urllib.parse._to_bytes('http://www.python.org')
        self.assertEqual(result, 'http://www.python.org')
        self.assertRaises(UnicodeError, urllib.parse._to_bytes,
                          'http://www.python.org/medi\u00e6val')

    def test_unwrap(self):
        for wrapped_url in ('<URL:scheme://host/path>', '<scheme://host/path>',
                            'URL:scheme://host/path', 'scheme://host/path'):
            url = urllib.parse.unwrap(wrapped_url)
            self.assertEqual(url, 'scheme://host/path')


class DeprecationTest(unittest.TestCase):

    def test_Quoter_deprecation(self):
        with self.assertWarns(DeprecationWarning) as cm:
            old_class = urllib.parse.Quoter
            self.assertIs(old_class, urllib.parse._Quoter)
        self.assertIn('Quoter will be removed', str(cm.warning))

    def test_splittype_deprecation(self):
        with self.assertWarns(DeprecationWarning) as cm:
            urllib.parse.splittype('')
        self.assertEqual(str(cm.warning),
                         'urllib.parse.splittype() is deprecated as of 3.8, '
                         'use urllib.parse.urlparse() instead')

    def test_splithost_deprecation(self):
        with self.assertWarns(DeprecationWarning) as cm:
            urllib.parse.splithost('')
        self.assertEqual(str(cm.warning),
                         'urllib.parse.splithost() is deprecated as of 3.8, '
                         'use urllib.parse.urlparse() instead')

    def test_splituser_deprecation(self):
        with self.assertWarns(DeprecationWarning) as cm:
            urllib.parse.splituser('')
        self.assertEqual(str(cm.warning),
                         'urllib.parse.splituser() is deprecated as of 3.8, '
                         'use urllib.parse.urlparse() instead')

    def test_splitpasswd_deprecation(self):
        with self.assertWarns(DeprecationWarning) as cm:
            urllib.parse.splitpasswd('')
        self.assertEqual(str(cm.warning),
                         'urllib.parse.splitpasswd() is deprecated as of 3.8, '
                         'use urllib.parse.urlparse() instead')

    def test_splitport_deprecation(self):
        with self.assertWarns(DeprecationWarning) as cm:
            urllib.parse.splitport('')
        self.assertEqual(str(cm.warning),
                         'urllib.parse.splitport() is deprecated as of 3.8, '
                         'use urllib.parse.urlparse() instead')

    def test_splitnport_deprecation(self):
        with self.assertWarns(DeprecationWarning) as cm:
            urllib.parse.splitnport('')
        self.assertEqual(str(cm.warning),
                         'urllib.parse.splitnport() is deprecated as of 3.8, '
                         'use urllib.parse.urlparse() instead')

    def test_splitquery_deprecation(self):
        with self.assertWarns(DeprecationWarning) as cm:
            urllib.parse.splitquery('')
        self.assertEqual(str(cm.warning),
                         'urllib.parse.splitquery() is deprecated as of 3.8, '
                         'use urllib.parse.urlparse() instead')

    def test_splittag_deprecation(self):
        with self.assertWarns(DeprecationWarning) as cm:
            urllib.parse.splittag('')
        self.assertEqual(str(cm.warning),
                         'urllib.parse.splittag() is deprecated as of 3.8, '
                         'use urllib.parse.urlparse() instead')

    def test_splitattr_deprecation(self):
        with self.assertWarns(DeprecationWarning) as cm:
            urllib.parse.splitattr('')
        self.assertEqual(str(cm.warning),
                         'urllib.parse.splitattr() is deprecated as of 3.8, '
                         'use urllib.parse.urlparse() instead')

    def test_splitvalue_deprecation(self):
        with self.assertWarns(DeprecationWarning) as cm:
            urllib.parse.splitvalue('')
        self.assertEqual(str(cm.warning),
                         'urllib.parse.splitvalue() is deprecated as of 3.8, '
                         'use urllib.parse.parse_qsl() instead')

    def test_to_bytes_deprecation(self):
        with self.assertWarns(DeprecationWarning) as cm:
            urllib.parse.to_bytes('')
        self.assertEqual(str(cm.warning),
                         'urllib.parse.to_bytes() is deprecated as of 3.8')


if __name__ == "__main__":
    unittest.main()