    def test_getaddresses(self):
        eq = self.assertEqual
        eq(utils.getaddresses(['aperson@dom.ain (Al Person)',
                               'Bud Person <bperson@dom.ain>']),
           [('Al Person', 'aperson@dom.ain'),
            ('Bud Person', 'bperson@dom.ain')])

    def test_getaddresses_nasty(self):
        eq = self.assertEqual
        eq(utils.getaddresses(['foo: ;']), [('', '')])
        eq(utils.getaddresses(
           ['[]*-- =~$']),
           [('', ''), ('', ''), ('', '*--')])
        eq(utils.getaddresses(
           ['foo: ;', '"Jason R. Mastaler" <jason@dom.ain>']),
           [('', ''), ('Jason R. Mastaler', 'jason@dom.ain')])

    def test_getaddresses_embedded_comment(self):
        """Test proper handling of a nested comment"""
        eq = self.assertEqual
        addrs = utils.getaddresses(['User ((nested comment)) <foo@bar.com>'])
        eq(addrs[0][1], 'foo@bar.com')

    def test_getaddresses_header_obj(self):
        """Test the handling of a Header object."""
        addrs = utils.getaddresses([Header('Al Person <aperson@dom.ain>')])
        self.assertEqual(addrs[0][1], 'aperson@dom.ain')

    @threading_helper.requires_working_threading()
    def test_make_msgid_collisions(self):
        # Test make_msgid uniqueness, even with multiple threads
        class MsgidsThread(Thread):
            def run(self):
                # generate msgids for 3 seconds
                self.msgids = []
                append = self.msgids.append
                make_msgid = utils.make_msgid
                clock = time.monotonic
                tfin = clock() + 3.0
                while clock() < tfin:
                    append(make_msgid(domain='testdomain-string'))

        threads = [MsgidsThread() for i in range(5)]
        with threading_helper.start_threads(threads):
            pass
        all_ids = sum([t.msgids for t in threads], [])
        self.assertEqual(len(set(all_ids)), len(all_ids))

    def test_utils_quote_unquote(self):
        eq = self.assertEqual
        msg = Message()
        msg.add_header('content-disposition', 'attachment',
                       filename='foo\\wacky"name')
        eq(msg.get_filename(), 'foo\\wacky"name')

    def test_get_body_encoding_with_bogus_charset(self):
        charset = Charset('not a charset')
        self.assertEqual(charset.get_body_encoding(), 'base64')

    def test_get_body_encoding_with_uppercase_charset(self):
        eq = self.assertEqual
        msg = Message()
        msg['Content-Type'] = 'text/plain; charset=UTF-8'
        eq(msg['content-type'], 'text/plain; charset=UTF-8')
        charsets = msg.get_charsets()
        eq(len(charsets), 1)
        eq(charsets[0], 'utf-8')
        charset = Charset(charsets[0])
        eq(charset.get_body_encoding(), 'base64')
        msg.set_payload(b'hello world', charset=charset)
        eq(msg.get_payload(), 'aGVsbG8gd29ybGQ=\n')
        eq(msg.get_payload(decode=True), b'hello world')
        eq(msg['content-transfer-encoding'], 'base64')
        # Try another one
        msg = Message()
        msg['Content-Type'] = 'text/plain; charset="US-ASCII"'
        charsets = msg.get_charsets()
        eq(len(charsets), 1)
        eq(charsets[0], 'us-ascii')
        charset = Charset(charsets[0])
        eq(charset.get_body_encoding(), encoders.encode_7or8bit)
        msg.set_payload('hello world', charset=charset)
        eq(msg.get_payload(), 'hello world')
        eq(msg['content-transfer-encoding'], '7bit')

    def test_charsets_case_insensitive(self):
        lc = Charset('us-ascii')
        uc = Charset('US-ASCII')
        self.assertEqual(lc.get_body_encoding(), uc.get_body_encoding())

    def test_partial_falls_inside_message_delivery_status(self):
        eq = self.ndiffAssertEqual
        # The Parser interface provides chunks of data to FeedParser in 8192
        # byte gulps.  SF bug #1076485 found one of those chunks inside
        # message/delivery-status header block, which triggered an
        # unreadline() of NeedMoreData.
        msg = self._msgobj('msg_43.txt')
        sfp = StringIO()
        iterators._structure(msg, sfp)
        eq(sfp.getvalue(), """\
multipart/report
    text/plain
    message/delivery-status
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
        text/plain
    text/rfc822-headers
""")

    def test_make_msgid_domain(self):
        self.assertEqual(
            email.utils.make_msgid(domain='testdomain-string')[-19:],
            '@testdomain-string>')

    def test_make_msgid_idstring(self):
        self.assertEqual(
            email.utils.make_msgid(idstring='test-idstring',
                domain='testdomain-string')[-33:],
            '.test-idstring@testdomain-string>')

    def test_make_msgid_default_domain(self):
        with patch('socket.getfqdn') as mock_getfqdn:
            mock_getfqdn.return_value = domain = 'pythontest.example.com'
            self.assertTrue(
                email.utils.make_msgid().endswith(
                    '@' + domain + '>'))

    def test_Generator_linend(self):
        # Issue 14645.
        with openfile('msg_26.txt', encoding="utf-8", newline='\n') as f:
            msgtxt = f.read()
        msgtxt_nl = msgtxt.replace('\r\n', '\n')
        msg = email.message_from_string(msgtxt)
        s = StringIO()
        g = email.generator.Generator(s)
        g.flatten(msg)
        self.assertEqual(s.getvalue(), msgtxt_nl)

    def test_BytesGenerator_linend(self):
        # Issue 14645.
        with openfile('msg_26.txt', encoding="utf-8", newline='\n') as f:
            msgtxt = f.read()
        msgtxt_nl = msgtxt.replace('\r\n', '\n')
        msg = email.message_from_string(msgtxt_nl)
        s = BytesIO()
        g = email.generator.BytesGenerator(s)
        g.flatten(msg, linesep='\r\n')
        self.assertEqual(s.getvalue().decode('ascii'), msgtxt)

    def test_BytesGenerator_linend_with_non_ascii(self):
        # Issue 14645.
        with openfile('msg_26.txt', 'rb') as f:
            msgtxt = f.read()
        msgtxt = msgtxt.replace(b'with attachment', b'fo\xf6')
        msgtxt_nl = msgtxt.replace(b'\r\n', b'\n')
        msg = email.message_from_bytes(msgtxt_nl)
        s = BytesIO()
        g = email.generator.BytesGenerator(s)
        g.flatten(msg, linesep='\r\n')
        self.assertEqual(s.getvalue(), msgtxt)

    def test_mime_classes_policy_argument(self):
        with openfile('sndhdr.au', 'rb') as fp:
            audiodata = fp.read()
        with openfile('python.gif', 'rb') as fp:
            bindata = fp.read()
        classes = [
            (MIMEApplication, ('',)),
            (MIMEAudio, (audiodata,)),
            (MIMEImage, (bindata,)),
            (MIMEMessage, (Message(),)),
            (MIMENonMultipart, ('multipart', 'mixed')),
            (MIMEText, ('',)),
        ]
        for cls, constructor in classes:
            with self.subTest(cls=cls.__name__, policy='compat32'):
                m = cls(*constructor)
                self.assertIs(m.policy, email.policy.compat32)
            with self.subTest(cls=cls.__name__, policy='default'):
                m = cls(*constructor, policy=email.policy.default)
                self.assertIs(m.policy, email.policy.default)


# Test the iterator/generators
class TestIterators(TestEmailBase):
    def test_body_line_iterator(self):
        eq = self.assertEqual
        neq = self.ndiffAssertEqual
        # First a simple non-multipart message
        msg = self._msgobj('msg_01.txt')
        it = iterators.body_line_iterator(msg)
        lines = list(it)
        eq(len(lines), 6)
        neq(EMPTYSTRING.join(lines), msg.get_payload())
        # Now a more complicated multipart
        msg = self._msgobj('msg_02.txt')
        it = iterators.body_line_iterator(msg)
        lines = list(it)
        eq(len(lines), 43)
        with openfile('msg_19.txt', encoding="utf-8") as fp:
            neq(EMPTYSTRING.join(lines), fp.read())

    def test_typed_subpart_iterator(self):
        eq = self.assertEqual
        msg = self._msgobj('msg_04.txt')
        it = iterators.typed_subpart_iterator(msg, 'text')
        lines = []
        subparts = 0
        for subpart in it:
            subparts += 1
            lines.append(subpart.get_payload())
        eq(subparts, 2)
        eq(EMPTYSTRING.join(lines), """\
a simple kind of mirror
to reflect upon our own
a simple kind of mirror
to reflect upon our own
""")

    def test_typed_subpart_iterator_default_type(self):
        eq = self.assertEqual
        msg = self._msgobj('msg_03.txt')
        it = iterators.typed_subpart_iterator(msg, 'text', 'plain')
        lines = []
        subparts = 0
        for subpart in it:
            subparts += 1
            lines.append(subpart.get_payload())
        eq(subparts, 1)
        eq(EMPTYSTRING.join(lines), """\

Hi,

Do you like this message?

-Me
""")

    def test_pushCR_LF(self):
        '''FeedParser BufferedSubFile.push() assumed it received complete
           line endings.  A CR ending one push() followed by a LF starting
           the next push() added an empty line.
        '''
        imt = [
            ("a\r \n",  2),
            ("b",       0),
            ("c\n",     1),
            ("",        0),
            ("d\r\n",   1),
            ("e\r",     0),
            ("\nf",     1),
            ("\r\n",    1),
          ]
        from email.feedparser import BufferedSubFile, NeedMoreData
        bsf = BufferedSubFile()
        om = []
        nt = 0
        for il, n in imt:
            bsf.push(il)
            nt += n
            n1 = 0
            for ol in iter(bsf.readline, NeedMoreData):
                om.append(ol)
                n1 += 1
            self.assertEqual(n, n1)
        self.assertEqual(len(om), nt)
        self.assertEqual(''.join([il for il, n in imt]), ''.join(om))

    def test_push_random(self):
        from email.feedparser import BufferedSubFile, NeedMoreData

        n = 10000
        chunksize = 5
        chars = 'abcd \t\r\n'

        s = ''.join(choice(chars) for i in range(n)) + '\n'
        target = s.splitlines(True)

        bsf = BufferedSubFile()
        lines = []
        for i in range(0, len(s), chunksize):
            chunk = s[i:i+chunksize]
            bsf.push(chunk)
            lines.extend(iter(bsf.readline, NeedMoreData))
        self.assertEqual(lines, target)


class TestFeedParsers(TestEmailBase):

    def parse(self, chunks):
        feedparser = FeedParser()
        for chunk in chunks:
            feedparser.feed(chunk)
        return feedparser.close()

    def test_empty_header_name_handled(self):
        # Issue 19996
        msg = self.parse("First: val\n: bad\nSecond: val")
        self.assertEqual(msg['First'], 'val')
        self.assertEqual(msg['Second'], 'val')

    def test_newlines(self):
        m = self.parse(['a:\nb:\rc:\r\nd:\n'])
        self.assertEqual(m.keys(), ['a', 'b', 'c', 'd'])
        m = self.parse(['a:\nb:\rc:\r\nd:'])
        self.assertEqual(m.keys(), ['a', 'b', 'c', 'd'])
        m = self.parse(['a:\rb', 'c:\n'])
        self.assertEqual(m.keys(), ['a', 'bc'])
        m = self.parse(['a:\r', 'b:\n'])
        self.assertEqual(m.keys(), ['a', 'b'])
        m = self.parse(['a:\r', '\nb:\n'])
        self.assertEqual(m.keys(), ['a', 'b'])

        # Only CR and LF should break header fields
        m = self.parse(['a:\x85b:\u2028c:\n'])
        self.assertEqual(m.items(), [('a', '\x85b:\u2028c:')])
        m = self.parse(['a:\r', 'b:\x85', 'c:\n'])
        self.assertEqual(m.items(), [('a', ''), ('b', '\x85c:')])

    def test_long_lines(self):
        # Expected peak memory use on 32-bit platform: 6*N*M bytes.
        M, N = 1000, 20000
        m = self.parse(['a:b\n\n'] + ['x'*M] * N)
        self.assertEqual(m.items(), [('a', 'b')])
        self.assertEqual(m.get_payload(), 'x'*M*N)
        m = self.parse(['a:b\r\r'] + ['x'*M] * N)
        self.assertEqual(m.items(), [('a', 'b')])
        self.assertEqual(m.get_payload(), 'x'*M*N)
        m = self.parse(['a:b\r\r'] + ['x'*M+'\x85'] * N)
        self.assertEqual(m.items(), [('a', 'b')])
        self.assertEqual(m.get_payload(), ('x'*M+'\x85')*N)
        m = self.parse(['a:\r', 'b: '] + ['x'*M] * N)
        self.assertEqual(m.items(), [('a', ''), ('b', 'x'*M*N)])


class TestParsers(TestEmailBase):

    def test_header_parser(self):
        eq = self.assertEqual
        # Parse only the headers of a complex multipart MIME document
        with openfile('msg_02.txt', encoding="utf-8") as fp:
            msg = HeaderParser().parse(fp)
        eq(msg['from'], 'ppp-request@zzz.org')
        eq(msg['to'], 'ppp@zzz.org')
        eq(msg.get_content_type(), 'multipart/mixed')
        self.assertFalse(msg.is_multipart())
        self.assertIsInstance(msg.get_payload(), str)

    def test_bytes_header_parser(self):
        eq = self.assertEqual
        # Parse only the headers of a complex multipart MIME document
        with openfile('msg_02.txt', 'rb') as fp:
            msg = email.parser.BytesHeaderParser().parse(fp)
        eq(msg['from'], 'ppp-request@zzz.org')
        eq(msg['to'], 'ppp@zzz.org')
        eq(msg.get_content_type(), 'multipart/mixed')
        self.assertFalse(msg.is_multipart())
        self.assertIsInstance(msg.get_payload(), str)
        self.assertIsInstance(msg.get_payload(decode=True), bytes)

    def test_bytes_parser_does_not_close_file(self):
        with openfile('msg_02.txt', 'rb') as fp:
            email.parser.BytesParser().parse(fp)
            self.assertFalse(fp.closed)

    def test_bytes_parser_on_exception_does_not_close_file(self):
        with openfile('msg_15.txt', 'rb') as fp:
            bytesParser = email.parser.BytesParser
            self.assertRaises(email.errors.StartBoundaryNotFoundDefect,
                              bytesParser(policy=email.policy.strict).parse,
                              fp)
            self.assertFalse(fp.closed)

    def test_parser_does_not_close_file(self):
        with openfile('msg_02.txt', encoding="utf-8") as fp:
            email.parser.Parser().parse(fp)
            self.assertFalse(fp.closed)

    def test_parser_on_exception_does_not_close_file(self):
        with openfile('msg_15.txt', encoding="utf-8") as fp:
            parser = email.parser.Parser
            self.assertRaises(email.errors.StartBoundaryNotFoundDefect,
                              parser(policy=email.policy.strict).parse, fp)
            self.assertFalse(fp.closed)

    def test_whitespace_continuation(self):
        eq = self.assertEqual
        # This message contains a line after the Subject: header that has only
        # whitespace, but it is not empty!
        msg = email.message_from_string("""\
From: aperson@dom.ain
To: bperson@dom.ain
Subject: the next line has a space on it
\x20
Date: Mon, 8 Apr 2002 15:09:19 -0400
Message-ID: spam

Here's the message body
""")
        eq(msg['subject'], 'the next line has a space on it\n ')
        eq(msg['message-id'], 'spam')
        eq(msg.get_payload(), "Here's the message body\n")

    def test_whitespace_continuation_last_header(self):
        eq = self.assertEqual
        # Like the previous test, but the subject line is the last
        # header.
        msg = email.message_from_string("""\
From: aperson@dom.ain
To: bperson@dom.ain
Date: Mon, 8 Apr 2002 15:09:19 -0400
Message-ID: spam
Subject: the next line has a space on it
\x20

Here's the message body
""")
        eq(msg['subject'], 'the next line has a space on it\n ')
        eq(msg['message-id'], 'spam')
        eq(msg.get_payload(), "Here's the message body\n")

    def test_crlf_separation(self):
        eq = self.assertEqual
        with openfile('msg_26.txt', encoding="utf-8", newline='\n') as fp:
            msg = Parser().parse(fp)
        eq(len(msg.get_payload()), 2)
        part1 = msg.get_payload(0)
        eq(part1.get_content_type(), 'text/plain')
        eq(part1.get_payload(), 'Simple email with attachment.\r\n\r\n')
        part2 = msg.get_payload(1)
        eq(part2.get_content_type(), 'application/riscos')

    def test_crlf_flatten(self):
        # Using newline='\n' preserves the crlfs in this input file.
        with openfile('msg_26.txt', encoding="utf-8", newline='\n') as fp:
            text = fp.read()
        msg = email.message_from_string(text)
        s = StringIO()
        g = Generator(s)
        g.flatten(msg, linesep='\r\n')
        self.assertEqual(s.getvalue(), text)

    maxDiff = None

    def test_multipart_digest_with_extra_mime_headers(self):
        eq = self.assertEqual
        neq = self.ndiffAssertEqual
        with openfile('msg_28.txt', encoding="utf-8") as fp:
            msg = email.message_from_file(fp)
        # Structure is:
        # multipart/digest
        #   message/rfc822
        #     text/plain
        #   message/rfc822
        #     text/plain
        eq(msg.is_multipart(), 1)
        eq(len(msg.get_payload()), 2)
        part1 = msg.get_payload(0)
        eq(part1.get_content_type(), 'message/rfc822')
        eq(part1.is_multipart(), 1)
        eq(len(part1.get_payload()), 1)
        part1a = part1.get_payload(0)
        eq(part1a.is_multipart(), 0)
        eq(part1a.get_content_type(), 'text/plain')
        neq(part1a.get_payload(), 'message 1\n')
        # next message/rfc822
        part2 = msg.get_payload(1)
        eq(part2.get_content_type(), 'message/rfc822')
        eq(part2.is_multipart(), 1)
        eq(len(part2.get_payload()), 1)
        part2a = part2.get_payload(0)
        eq(part2a.is_multipart(), 0)
        eq(part2a.get_content_type(), 'text/plain')
        neq(part2a.get_payload(), 'message 2\n')

    def test_three_lines(self):
        # A bug report by Andrew McNamara
        lines = ['From: Andrew Person <aperson@dom.ain',
                 'Subject: Test',
                 'Date: Tue, 20 Aug 2002 16:43:45 +1000']
        msg = email.message_from_string(NL.join(lines))
        self.assertEqual(msg['date'], 'Tue, 20 Aug 2002 16:43:45 +1000')

    def test_strip_line_feed_and_carriage_return_in_headers(self):
        eq = self.assertEqual
        # For [ 1002475 ] email message parser doesn't handle \r\n correctly
        value1 = 'text'
        value2 = 'more text'
        m = 'Header: %s\r\nNext-Header: %s\r\n\r\nBody\r\n\r\n' % (
            value1, value2)
        msg = email.message_from_string(m)
        eq(msg.get('Header'), value1)
        eq(msg.get('Next-Header'), value2)

    def test_rfc2822_header_syntax(self):
        eq = self.assertEqual
        m = '>From: foo\nFrom: bar\n!"#QUX;~: zoo\n\nbody'
        msg = email.message_from_string(m)
        eq(len(msg), 3)
        eq(sorted(field for field in msg), ['!"#QUX;~', '>From', 'From'])
        eq(msg.get_payload(), 'body')

    def test_rfc2822_space_not_allowed_in_header(self):
        eq = self.assertEqual
        m = '>From foo@example.com 11:25:53\nFrom: bar\n!"#QUX;~: zoo\n\nbody'
        msg = email.message_from_string(m)
        eq(len(msg.keys()), 0)

    def test_rfc2822_one_character_header(self):
        eq = self.assertEqual
        m = 'A: first header\nB: second header\nCC: third header\n\nbody'
        msg = email.message_from_string(m)
        headers = msg.keys()
        headers.sort()
        eq(headers, ['A', 'B', 'CC'])
        eq(msg.get_payload(), 'body')

    def test_CRLFLF_at_end_of_part(self):
        # issue 5610: feedparser should not eat two chars from body part ending
        # with "\r\n\n".
        m = (
            "From: foo@bar.com\n"
            "To: baz\n"
            "Mime-Version: 1.0\n"
            "Content-Type: multipart/mixed; boundary=BOUNDARY\n"
            "\n"
            "--BOUNDARY\n"
            "Content-Type: text/plain\n"
            "\n"
            "body ending with CRLF newline\r\n"
            "\n"
            "--BOUNDARY--\n"
          )
        msg = email.message_from_string(m)
        self.assertTrue(msg.get_payload(0).get_payload().endswith('\r\n'))


class Test8BitBytesHandling(TestEmailBase):
    # In Python3 all input is string, but that doesn't work if the actual input
    # uses an 8bit transfer encoding.  To hack around that, in email 5.1 we
    # decode byte streams using the surrogateescape error handler, and
    # reconvert to binary at appropriate places if we detect surrogates.  This
    # doesn't allow us to transform headers with 8bit bytes (they get munged),
    # but it does allow us to parse and preserve them, and to decode body
    # parts that use an 8bit CTE.

    bodytest_msg = textwrap.dedent("""\
        From: foo@bar.com
        To: baz
        Mime-Version: 1.0
        Content-Type: text/plain; charset={charset}
        Content-Transfer-Encoding: {cte}

        {bodyline}
        """)

    def test_known_8bit_CTE(self):
        m = self.bodytest_msg.format(charset='utf-8',
                                     cte='8bit',
                                     bodyline='pöstal').encode('utf-8')
        msg = email.message_from_bytes(m)
        self.assertEqual(msg.get_payload(), "pöstal\n")
        self.assertEqual(msg.get_payload(decode=True),
                         "pöstal\n".encode('utf-8'))

    def test_unknown_8bit_CTE(self):
        m = self.bodytest_msg.format(charset='notavalidcharset',
                                     cte='8bit',
                                     bodyline='pöstal').encode('utf-8')
        msg = email.message_from_bytes(m)
        self.assertEqual(msg.get_payload(), "p\uFFFD\uFFFDstal\n")
        self.assertEqual(msg.get_payload(decode=True),
                         "pöstal\n".encode('utf-8'))

    def test_8bit_in_quopri_body(self):
        # This is non-RFC compliant data...without 'decode' the library code
        # decodes the body using the charset from the headers, and because the
        # source byte really is utf-8 this works.  This is likely to fail
        # against real dirty data (ie: produce mojibake), but the data is
        # invalid anyway so it is as good a guess as any.  But this means that
        # this test just confirms the current behavior; that behavior is not
        # necessarily the best possible behavior.  With 'decode' it is
        # returning the raw bytes, so that test should be of correct behavior,
        # or at least produce the same result that email4 did.
        m = self.bodytest_msg.format(charset='utf-8',
                                     cte='quoted-printable',
                                     bodyline='p=C3=B6stál').encode('utf-8')
        msg = email.message_from_bytes(m)
        self.assertEqual(msg.get_payload(), 'p=C3=B6stál\n')
        self.assertEqual(msg.get_payload(decode=True),
                         'pöstál\n'.encode('utf-8'))

    def test_invalid_8bit_in_non_8bit_cte_uses_replace(self):
        # This is similar to the previous test, but proves that if the 8bit
        # byte is undecodeable in the specified charset, it gets replaced
        # by the unicode 'unknown' character.  Again, this may or may not
        # be the ideal behavior.  Note that if decode=False none of the
        # decoders will get involved, so this is the only test we need
        # for this behavior.
        m = self.bodytest_msg.format(charset='ascii',
                                     cte='quoted-printable',
                                     bodyline='p=C3=B6stál').encode('utf-8')
        msg = email.message_from_bytes(m)
        self.assertEqual(msg.get_payload(), 'p=C3=B6st\uFFFD\uFFFDl\n')
        self.assertEqual(msg.get_payload(decode=True),
                        'pöstál\n'.encode('utf-8'))

    # test_defect_handling:test_invalid_chars_in_base64_payload
    def test_8bit_in_base64_body(self):
        # If we get 8bit bytes in a base64 body, we can just ignore them
        # as being outside the base64 alphabet and decode anyway.  But
        # we register a defect.
        m = self.bodytest_msg.format(charset='utf-8',
                                     cte='base64',
                                     bodyline='cMO2c3RhbAá=').encode('utf-8')
        msg = email.message_from_bytes(m)
        self.assertEqual(msg.get_payload(decode=True),
                         'pöstal'.encode('utf-8'))
        self.assertIsInstance(msg.defects[0],
                              errors.InvalidBase64CharactersDefect)

    def test_8bit_in_uuencode_body(self):
        # Sticking an 8bit byte in a uuencode block makes it undecodable by
        # normal means, so the block is returned undecoded, but as bytes.
        m = self.bodytest_msg.format(charset='utf-8',
                                     cte='uuencode',
                                     bodyline='<,.V<W1A; á ').encode('utf-8')
        msg = email.message_from_bytes(m)
        self.assertEqual(msg.get_payload(decode=True),
                         '<,.V<W1A; á \n'.encode('utf-8'))


    headertest_headers = (
        ('From: foo@bar.com', ('From', 'foo@bar.com')),
        ('To: báz', ('To', '=?unknown-8bit?q?b=C3=A1z?=')),
        ('Subject: Maintenant je vous présente mon collègue, le pouf célèbre\n'
            '\tJean de Baddie',
            ('Subject', '=?unknown-8bit?q?Maintenant_je_vous_pr=C3=A9sente_mon_'
                'coll=C3=A8gue=2C_le_pouf_c=C3=A9l=C3=A8bre?=\n'
                ' =?unknown-8bit?q?_Jean_de_Baddie?=')),
        ('From: göst', ('From', '=?unknown-8bit?b?Z8O2c3Q=?=')),
        )
    headertest_msg = ('\n'.join([src for (src, _) in headertest_headers]) +
        '\nYes, they are flying.\n').encode('utf-8')

    def test_get_8bit_header(self):
        msg = email.message_from_bytes(self.headertest_msg)
        self.assertEqual(str(msg.get('to')), 'b\uFFFD\uFFFDz')
        self.assertEqual(str(msg['to']), 'b\uFFFD\uFFFDz')

    def test_print_8bit_headers(self):
        msg = email.message_from_bytes(self.headertest_msg)
        self.assertEqual(str(msg),
                         textwrap.dedent("""\
                            From: {}
                            To: {}
                            Subject: {}
                            From: {}

                            Yes, they are flying.
                            """).format(*[expected[1] for (_, expected) in
                                        self.headertest_headers]))

    def test_values_with_8bit_headers(self):
        msg = email.message_from_bytes(self.headertest_msg)
        self.assertListEqual([str(x) for x in msg.values()],
                              ['foo@bar.com',
                               'b\uFFFD\uFFFDz',
                               'Maintenant je vous pr\uFFFD\uFFFDsente mon '
                                   'coll\uFFFD\uFFFDgue, le pouf '
                                   'c\uFFFD\uFFFDl\uFFFD\uFFFDbre\n'
                                   '\tJean de Baddie',
                               "g\uFFFD\uFFFDst"])

    def test_items_with_8bit_headers(self):
        msg = email.message_from_bytes(self.headertest_msg)
        self.assertListEqual([(str(x), str(y)) for (x, y) in msg.items()],
                              [('From', 'foo@bar.com'),
                               ('To', 'b\uFFFD\uFFFDz'),
                               ('Subject', 'Maintenant je vous '
                                  'pr\uFFFD\uFFFDsente '
                                  'mon coll\uFFFD\uFFFDgue, le pouf '
                                  'c\uFFFD\uFFFDl\uFFFD\uFFFDbre\n'
                                  '\tJean de Baddie'),
                               ('From', 'g\uFFFD\uFFFDst')])

    def test_get_all_with_8bit_headers(self):
        msg = email.message_from_bytes(self.headertest_msg)
        self.assertListEqual([str(x) for x in msg.get_all('from')],
                              ['foo@bar.com',
                               'g\uFFFD\uFFFDst'])

    def test_get_content_type_with_8bit(self):
        msg = email.message_from_bytes(textwrap.dedent("""\
            Content-Type: text/pl\xA7in; charset=utf-8
            """).encode('latin-1'))
        self.assertEqual(msg.get_content_type(), "text/pl\uFFFDin")
        self.assertEqual(msg.get_content_maintype(), "text")
        self.assertEqual(msg.get_content_subtype(), "pl\uFFFDin")

    # test_headerregistry.TestContentTypeHeader.non_ascii_in_params
    def test_get_params_with_8bit(self):
        msg = email.message_from_bytes(
            'X-Header: foo=\xa7ne; b\xa7r=two; baz=three\n'.encode('latin-1'))
        self.assertEqual(msg.get_params(header='x-header'),
           [('foo', '\uFFFDne'), ('b\uFFFDr', 'two'), ('baz', 'three')])
        self.assertEqual(msg.get_param('Foo', header='x-header'), '\uFFFdne')
        # XXX: someday you might be able to get 'b\xa7r', for now you can't.
        self.assertEqual(msg.get_param('b\xa7r', header='x-header'), None)

    # test_headerregistry.TestContentTypeHeader.non_ascii_in_rfc2231_value
    def test_get_rfc2231_params_with_8bit(self):
        msg = email.message_from_bytes(textwrap.dedent("""\
            Content-Type: text/plain; charset=us-ascii;
             title*=us-ascii'en'This%20is%20not%20f\xa7n"""
             ).encode('latin-1'))
        self.assertEqual(msg.get_param('title'),
            ('us-ascii', 'en', 'This is not f\uFFFDn'))

    def test_set_rfc2231_params_with_8bit(self):
        msg = email.message_from_bytes(textwrap.dedent("""\
            Content-Type: text/plain; charset=us-ascii;
             title*=us-ascii'en'This%20is%20not%20f\xa7n"""
             ).encode('latin-1'))
        msg.set_param('title', 'test')
        self.assertEqual(msg.get_param('title'), 'test')

    def test_del_rfc2231_params_with_8bit(self):
        msg = email.message_from_bytes(textwrap.dedent("""\
            Content-Type: text/plain; charset=us-ascii;
             title*=us-ascii'en'This%20is%20not%20f\xa7n"""
             ).encode('latin-1'))
        msg.del_param('title')
        self.assertEqual(msg.get_param('title'), None)
        self.assertEqual(msg.get_content_maintype(), 'text')

    def test_get_payload_with_8bit_cte_header(self):
        msg = email.message_from_bytes(textwrap.dedent("""\
            Content-Transfer-Encoding: b\xa7se64
            Content-Type: text/plain; charset=latin-1

            payload
            """).encode('latin-1'))
        self.assertEqual(msg.get_payload(), 'payload\n')
        self.assertEqual(msg.get_payload(decode=True), b'payload\n')

    non_latin_bin_msg = textwrap.dedent("""\
        From: foo@bar.com
        To: báz
        Subject: Maintenant je vous présente mon collègue, le pouf célèbre
        \tJean de Baddie
        Mime-Version: 1.0
        Content-Type: text/plain; charset="utf-8"
        Content-Transfer-Encoding: 8bit

        Да, они летят.
        """).encode('utf-8')

    def test_bytes_generator(self):
        msg = email.message_from_bytes(self.non_latin_bin_msg)
        out = BytesIO()
        email.generator.BytesGenerator(out).flatten(msg)
        self.assertEqual(out.getvalue(), self.non_latin_bin_msg)

    def test_bytes_generator_handles_None_body(self):
        #Issue 11019
        msg = email.message.Message()
        out = BytesIO()
        email.generator.BytesGenerator(out).flatten(msg)
        self.assertEqual(out.getvalue(), b"\n")

    non_latin_bin_msg_as7bit_wrapped = textwrap.dedent("""\
        From: foo@bar.com
        To: =?unknown-8bit?q?b=C3=A1z?=
        Subject: =?unknown-8bit?q?Maintenant_je_vous_pr=C3=A9sente_mon_coll=C3=A8gue?=
         =?unknown-8bit?q?=2C_le_pouf_c=C3=A9l=C3=A8bre?=
         =?unknown-8bit?q?_Jean_de_Baddie?=
        Mime-Version: 1.0
        Content-Type: text/plain; charset="utf-8"
        Content-Transfer-Encoding: base64

        0JTQsCwg0L7QvdC4INC70LXRgtGP0YIuCg==
        """)

    def test_generator_handles_8bit(self):
        msg = email.message_from_bytes(self.non_latin_bin_msg)
        out = StringIO()
        email.generator.Generator(out).flatten(msg)
        self.assertEqual(out.getvalue(), self.non_latin_bin_msg_as7bit_wrapped)

    def test_str_generator_should_not_mutate_msg_when_handling_8bit(self):
        msg = email.message_from_bytes(self.non_latin_bin_msg)
        out = BytesIO()
        BytesGenerator(out).flatten(msg)
        orig_value = out.getvalue()
        Generator(StringIO()).flatten(msg) # Should not mutate msg!
        out = BytesIO()
        BytesGenerator(out).flatten(msg)
        self.assertEqual(out.getvalue(), orig_value)

    def test_bytes_generator_with_unix_from(self):
        # The unixfrom contains a current date, so we can't check it
        # literally.  Just make sure the first word is 'From' and the
        # rest of the message matches the input.
        msg = email.message_from_bytes(self.non_latin_bin_msg)
        out = BytesIO()
        email.generator.BytesGenerator(out).flatten(msg, unixfrom=True)
        lines = out.getvalue().split(b'\n')
        self.assertEqual(lines[0].split()[0], b'From')
        self.assertEqual(b'\n'.join(lines[1:]), self.non_latin_bin_msg)

    non_latin_bin_msg_as7bit = non_latin_bin_msg_as7bit_wrapped.split('\n')
    non_latin_bin_msg_as7bit[2:4] = [
        'Subject: =?unknown-8bit?q?Maintenant_je_vous_pr=C3=A9sente_mon_'
         'coll=C3=A8gue=2C_le_pouf_c=C3=A9l=C3=A8bre?=']
    non_latin_bin_msg_as7bit = '\n'.join(non_latin_bin_msg_as7bit)

    def test_message_from_binary_file(self):
        fn = 'test.msg'
        self.addCleanup(unlink, fn)
        with open(fn, 'wb') as testfile:
            testfile.write(self.non_latin_bin_msg)
        with open(fn, 'rb') as testfile:
            m = email.parser.BytesParser().parse(testfile)
        self.assertEqual(str(m), self.non_latin_bin_msg_as7bit)

    latin_bin_msg = textwrap.dedent("""\
        From: foo@bar.com
        To: Dinsdale
        Subject: Nudge nudge, wink, wink
        Mime-Version: 1.0
        Content-Type: text/plain; charset="latin-1"
        Content-Transfer-Encoding: 8bit

        oh là là, know what I mean, know what I mean?
        """).encode('latin-1')

    latin_bin_msg_as7bit = textwrap.dedent("""\
        From: foo@bar.com
        To: Dinsdale
        Subject: Nudge nudge, wink, wink
        Mime-Version: 1.0
        Content-Type: text/plain; charset="iso-8859-1"
        Content-Transfer-Encoding: quoted-printable

        oh l=E0 l=E0, know what I mean, know what I mean?
        """)

    def test_string_generator_reencodes_to_quopri_when_appropriate(self):
        m = email.message_from_bytes(self.latin_bin_msg)
        self.assertEqual(str(m), self.latin_bin_msg_as7bit)

    def test_decoded_generator_emits_unicode_body(self):
        m = email.message_from_bytes(self.latin_bin_msg)
        out = StringIO()
        email.generator.DecodedGenerator(out).flatten(m)
        #DecodedHeader output contains an extra blank line compared
        #to the input message.  RDM: not sure if this is a bug or not,
        #but it is not specific to the 8bit->7bit conversion.
        self.assertEqual(out.getvalue(),
            self.latin_bin_msg.decode('latin-1')+'\n')

    def test_bytes_feedparser(self):
        bfp = email.feedparser.BytesFeedParser()
        for i in range(0, len(self.latin_bin_msg), 10):
            bfp.feed(self.latin_bin_msg[i:i+10])
        m = bfp.close()
        self.assertEqual(str(m), self.latin_bin_msg_as7bit)

    def test_crlf_flatten(self):
        with openfile('msg_26.txt', 'rb') as fp:
            text = fp.read()
        msg = email.message_from_bytes(text)
        s = BytesIO()
        g = email.generator.BytesGenerator(s)
        g.flatten(msg, linesep='\r\n')
        self.assertEqual(s.getvalue(), text)

    def test_8bit_multipart(self):
        # Issue 11605
        source = textwrap.dedent("""\
            Date: Fri, 18 Mar 2011 17:15:43 +0100
            To: foo@example.com
            From: foodwatch-Newsletter <bar@example.com>
            Subject: Aktuelles zu Japan, Klonfleisch und Smiley-System
            Message-ID: <76a486bee62b0d200f33dc2ca08220ad@localhost.localdomain>
            MIME-Version: 1.0
            Content-Type: multipart/alternative;
                    boundary="b1_76a486bee62b0d200f33dc2ca08220ad"

            --b1_76a486bee62b0d200f33dc2ca08220ad
            Content-Type: text/plain; charset="utf-8"
            Content-Transfer-Encoding: 8bit

            Guten Tag, ,

            mit großer Betroffenheit verfolgen auch wir im foodwatch-Team die
            Nachrichten aus Japan.


            --b1_76a486bee62b0d200f33dc2ca08220ad
            Content-Type: text/html; charset="utf-8"
            Content-Transfer-Encoding: 8bit

            <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
                "http://www.w3.org/TR/html4/loose.dtd">
            <html lang="de">
            <head>
                    <title>foodwatch - Newsletter</title>
            </head>
            <body>
              <p>mit gro&szlig;er Betroffenheit verfolgen auch wir im foodwatch-Team
                 die Nachrichten aus Japan.</p>
            </body>
            </html>
            --b1_76a486bee62b0d200f33dc2ca08220ad--

            """).encode('utf-8')
        msg = email.message_from_bytes(source)
        s = BytesIO()
        g = email.generator.BytesGenerator(s)
        g.flatten(msg)
        self.assertEqual(s.getvalue(), source)

    def test_bytes_generator_b_encoding_linesep(self):
        # Issue 14062: b encoding was tacking on an extra \n.
        m = Message()
        # This has enough non-ascii that it should always end up b encoded.
        m['Subject'] = Header('žluťoučký kůň')
        s = BytesIO()
        g = email.generator.BytesGenerator(s)
        g.flatten(m, linesep='\r\n')
        self.assertEqual(
            s.getvalue(),
            b'Subject: =?utf-8?b?xb5sdcWlb3XEjWvDvSBrxa/FiA==?=\r\n\r\n')

    def test_generator_b_encoding_linesep(self):
        # Since this broke in ByteGenerator, test Generator for completeness.
        m = Message()
        # This has enough non-ascii that it should always end up b encoded.
        m['Subject'] = Header('žluťoučký kůň')
        s = StringIO()
        g = email.generator.Generator(s)
        g.flatten(m, linesep='\r\n')
        self.assertEqual(
            s.getvalue(),
            'Subject: =?utf-8?b?xb5sdcWlb3XEjWvDvSBrxa/FiA==?=\r\n\r\n')

    maxDiff = None


class BaseTestBytesGeneratorIdempotent:

    maxDiff = None

    def _msgobj(self, filename):
        with openfile(filename, 'rb') as fp:
            data = fp.read()
        data = self.normalize_linesep_regex.sub(self.blinesep, data)
        msg = email.message_from_bytes(data)
        return msg, data

    def _idempotent(self, msg, data, unixfrom=False):
        b = BytesIO()
        g = email.generator.BytesGenerator(b, maxheaderlen=0)
        g.flatten(msg, unixfrom=unixfrom, linesep=self.linesep)
        self.assertEqual(data, b.getvalue())


class TestBytesGeneratorIdempotentNL(BaseTestBytesGeneratorIdempotent,
                                    TestIdempotent):
    linesep = '\n'
    blinesep = b'\n'
    normalize_linesep_regex = re.compile(br'\r\n')


class TestBytesGeneratorIdempotentCRLF(BaseTestBytesGeneratorIdempotent,
                                       TestIdempotent):
    linesep = '\r\n'
    blinesep = b'\r\n'
    normalize_linesep_regex = re.compile(br'(?<!\r)\n')


class TestBase64(unittest.TestCase):
    def test_len(self):
        eq = self.assertEqual
        eq(base64mime.header_length('hello'),
           len(base64mime.body_encode(b'hello', eol='')))
        for size in range(15):
            if   size == 0 : bsize = 0
            elif size <= 3 : bsize = 4
            elif size <= 6 : bsize = 8
            elif size <= 9 : bsize = 12
            elif size <= 12: bsize = 16
            else           : bsize = 20
            eq(base64mime.header_length('x' * size), bsize)

    def test_decode(self):
        eq = self.assertEqual
        eq(base64mime.decode(''), b'')
        eq(base64mime.decode('aGVsbG8='), b'hello')

    def test_encode(self):
        eq = self.assertEqual
        eq(base64mime.body_encode(b''), '')
        eq(base64mime.body_encode(b'hello'), 'aGVsbG8=\n')
        # Test the binary flag
        eq(base64mime.body_encode(b'hello\n'), 'aGVsbG8K\n')
        # Test the maxlinelen arg
        eq(base64mime.body_encode(b'xxxx ' * 20, maxlinelen=40), """\
eHh4eCB4eHh4IHh4eHggeHh4eCB4eHh4IHh4eHgg
eHh4eCB4eHh4IHh4eHggeHh4eCB4eHh4IHh4eHgg
eHh4eCB4eHh4IHh4eHggeHh4eCB4eHh4IHh4eHgg
eHh4eCB4eHh4IA==
""")
        # Test the eol argument
        eq(base64mime.body_encode(b'xxxx ' * 20, maxlinelen=40, eol='\r\n'),
           """\
eHh4eCB4eHh4IHh4eHggeHh4eCB4eHh4IHh4eHgg\r
eHh4eCB4eHh4IHh4eHggeHh4eCB4eHh4IHh4eHgg\r
eHh4eCB4eHh4IHh4eHggeHh4eCB4eHh4IHh4eHgg\r
eHh4eCB4eHh4IA==\r
""")

    def test_header_encode(self):
        eq = self.assertEqual
        he = base64mime.header_encode
        eq(he('hello'), '=?iso-8859-1?b?aGVsbG8=?=')
        eq(he('hello\r\nworld'), '=?iso-8859-1?b?aGVsbG8NCndvcmxk?=')
        eq(he('hello\nworld'), '=?iso-8859-1?b?aGVsbG8Kd29ybGQ=?=')
        # Test the charset option
        eq(he('hello', charset='iso-8859-2'), '=?iso-8859-2?b?aGVsbG8=?=')
        eq(he('hello\nworld'), '=?iso-8859-1?b?aGVsbG8Kd29ybGQ=?=')


class TestQuopri(unittest.TestCase):
    def setUp(self):
        # Set of characters (as byte integers) that don't need to be encoded
        # in headers.
        self.hlit = list(chain(
            range(ord('a'), ord('z') + 1),
            range(ord('A'), ord('Z') + 1),
            range(ord('0'), ord('9') + 1),
            (c for c in b'!*+-/')))
        # Set of characters (as byte integers) that do need to be encoded in
        # headers.
        self.hnon = [c for c in range(256) if c not in self.hlit]
        assert len(self.hlit) + len(self.hnon) == 256
        # Set of characters (as byte integers) that don't need to be encoded
        # in bodies.
        self.blit = list(range(ord(' '), ord('~') + 1))
        self.blit.append(ord('\t'))
        self.blit.remove(ord('='))
        # Set of characters (as byte integers) that do need to be encoded in
        # bodies.
        self.bnon = [c for c in range(256) if c not in self.blit]
        assert len(self.blit) + len(self.bnon) == 256

    def test_quopri_header_check(self):
        for c in self.hlit:
            self.assertFalse(quoprimime.header_check(c),
                        'Should not be header quopri encoded: %s' % chr(c))
        for c in self.hnon:
            self.assertTrue(quoprimime.header_check(c),
                            'Should be header quopri encoded: %s' % chr(c))

    def test_quopri_body_check(self):
        for c in self.blit:
            self.assertFalse(quoprimime.body_check(c),
                        'Should not be body quopri encoded: %s' % chr(c))
        for c in self.bnon:
            self.assertTrue(quoprimime.body_check(c),
                            'Should be body quopri encoded: %s' % chr(c))

    def test_header_quopri_len(self):
        eq = self.assertEqual
        eq(quoprimime.header_length(b'hello'), 5)
        # RFC 2047 chrome is not included in header_length().
        eq(len(quoprimime.header_encode(b'hello', charset='xxx')),
           quoprimime.header_length(b'hello') +
           # =?xxx?q?...?= means 10 extra characters
           10)
        eq(quoprimime.header_length(b'h@e@l@l@o@'), 20)
        # RFC 2047 chrome is not included in header_length().
        eq(len(quoprimime.header_encode(b'h@e@l@l@o@', charset='xxx')),
           quoprimime.header_length(b'h@e@l@l@o@') +
           # =?xxx?q?...?= means 10 extra characters
           10)
        for c in self.hlit:
            eq(quoprimime.header_length(bytes([c])), 1,
               'expected length 1 for %r' % chr(c))
        for c in self.hnon:
            # Space is special; it's encoded to _
            if c == ord(' '):
                continue
            eq(quoprimime.header_length(bytes([c])), 3,
               'expected length 3 for %r' % chr(c))
        eq(quoprimime.header_length(b' '), 1)

    def test_body_quopri_len(self):
        eq = self.assertEqual
        for c in self.blit:
            eq(quoprimime.body_length(bytes([c])), 1)
        for c in self.bnon:
            eq(quoprimime.body_length(bytes([c])), 3)

    def test_quote_unquote_idempotent(self):
        for x in range(256):
            c = chr(x)
            self.assertEqual(quoprimime.unquote(quoprimime.quote(c)), c)

    def _test_header_encode(self, header, expected_encoded_header, charset=None):
        if charset is None:
            encoded_header = quoprimime.header_encode(header)
        else:
            encoded_header = quoprimime.header_encode(header, charset)
        self.assertEqual(encoded_header, expected_encoded_header)

    def test_header_encode_null(self):
        self._test_header_encode(b'', '')

    def test_header_encode_one_word(self):
        self._test_header_encode(b'hello', '=?iso-8859-1?q?hello?=')

    def test_header_encode_two_lines(self):
        self._test_header_encode(b'hello\nworld',
                                '=?iso-8859-1?q?hello=0Aworld?=')

    def test_header_encode_non_ascii(self):
        self._test_header_encode(b'hello\xc7there',
                                '=?iso-8859-1?q?hello=C7there?=')

    def test_header_encode_alt_charset(self):
        self._test_header_encode(b'hello', '=?iso-8859-2?q?hello?=',
                charset='iso-8859-2')

    def _test_header_decode(self, encoded_header, expected_decoded_header):
        decoded_header = quoprimime.header_decode(encoded_header)
        self.assertEqual(decoded_header, expected_decoded_header)

    def test_header_decode_null(self):
        self._test_header_decode('', '')

    def test_header_decode_one_word(self):
        self._test_header_decode('hello', 'hello')

    def test_header_decode_two_lines(self):
        self._test_header_decode('hello=0Aworld', 'hello\nworld')

    def test_header_decode_non_ascii(self):
        self._test_header_decode('hello=C7there', 'hello\xc7there')

    def test_header_decode_re_bug_18380(self):
        # Issue 18380: Call re.sub with a positional argument for flags in the wrong position
        self.assertEqual(quoprimime.header_decode('=30' * 257), '0' * 257)

    def _test_decode(self, encoded, expected_decoded, eol=None):
        if eol is None:
            decoded = quoprimime.decode(encoded)
        else:
            decoded = quoprimime.decode(encoded, eol=eol)
        self.assertEqual(decoded, expected_decoded)

    def test_decode_null_word(self):
        self._test_decode('', '')

    def test_decode_null_line_null_word(self):
        self._test_decode('\r\n', '\n')

    def test_decode_one_word(self):
        self._test_decode('hello', 'hello')

    def test_decode_one_word_eol(self):
        self._test_decode('hello', 'hello', eol='X')

    def test_decode_one_line(self):
        self._test_decode('hello\r\n', 'hello\n')

    def test_decode_one_line_lf(self):
        self._test_decode('hello\n', 'hello\n')

    def test_decode_one_line_cr(self):
        self._test_decode('hello\r', 'hello\n')

    def test_decode_one_line_nl(self):
        self._test_decode('hello\n', 'helloX', eol='X')

    def test_decode_one_line_crnl(self):
        self._test_decode('hello\r\n', 'helloX', eol='X')

    def test_decode_one_line_one_word(self):
        self._test_decode('hello\r\nworld', 'hello\nworld')

    def test_decode_one_line_one_word_eol(self):
        self._test_decode('hello\r\nworld', 'helloXworld', eol='X')

    def test_decode_two_lines(self):
        self._test_decode('hello\r\nworld\r\n', 'hello\nworld\n')

    def test_decode_two_lines_eol(self):
        self._test_decode('hello\r\nworld\r\n', 'helloXworldX', eol='X')

    def test_decode_one_long_line(self):
        self._test_decode('Spam' * 250, 'Spam' * 250)

    def test_decode_one_space(self):
        self._test_decode(' ', '')

    def test_decode_multiple_spaces(self):
        self._test_decode(' ' * 5, '')

    def test_decode_one_line_trailing_spaces(self):
        self._test_decode('hello    \r\n', 'hello\n')

    def test_decode_two_lines_trailing_spaces(self):
        self._test_decode('hello    \r\nworld   \r\n', 'hello\nworld\n')

    def test_decode_quoted_word(self):
        self._test_decode('=22quoted=20words=22', '"quoted words"')

    def test_decode_uppercase_quoting(self):
        self._test_decode('ab=CD=EF', 'ab\xcd\xef')

    def test_decode_lowercase_quoting(self):
        self._test_decode('ab=cd=ef', 'ab\xcd\xef')

    def test_decode_soft_line_break(self):
        self._test_decode('soft line=\r\nbreak', 'soft linebreak')

    def test_decode_false_quoting(self):
        self._test_decode('A=1,B=A ==> A+B==2', 'A=1,B=A ==> A+B==2')

    def _test_encode(self, body, expected_encoded_body, maxlinelen=None, eol=None):
        kwargs = {}
        if maxlinelen is None:
            # Use body_encode's default.
            maxlinelen = 76
        else:
            kwargs['maxlinelen'] = maxlinelen
        if eol is None:
            # Use body_encode's default.
            eol = '\n'
        else:
            kwargs['eol'] = eol
        encoded_body = quoprimime.body_encode(body, **kwargs)
        self.assertEqual(encoded_body, expected_encoded_body)
        if eol == '\n' or eol == '\r\n':
            # We know how to split the result back into lines, so maxlinelen
            # can be checked.
            for line in encoded_body.splitlines():
                self.assertLessEqual(len(line), maxlinelen)

    def test_encode_null(self):
        self._test_encode('', '')

    def test_encode_null_lines(self):
        self._test_encode('\n\n', '\n\n')

    def test_encode_one_line(self):
        self._test_encode('hello\n', 'hello\n')

    def test_encode_one_line_crlf(self):
        self._test_encode('hello\r\n', 'hello\n')

    def test_encode_one_line_eol(self):
        self._test_encode('hello\n', 'hello\r\n', eol='\r\n')

    def test_encode_one_line_eol_after_non_ascii(self):
        # issue 20206; see changeset 0cf700464177 for why the encode/decode.
        self._test_encode('hello\u03c5\n'.encode('utf-8').decode('latin1'),
                          'hello=CF=85\r\n', eol='\r\n')

    def test_encode_one_space(self):
        self._test_encode(' ', '=20')

    def test_encode_one_line_one_space(self):
        self._test_encode(' \n', '=20\n')

# XXX: body_encode() expect strings, but uses ord(char) from these strings
# to index into a 256-entry list.  For code points above 255, this will fail.
# Should there be a check for 8-bit only ord() values in body, or at least
# a comment about the expected input?

    def test_encode_two_lines_one_space(self):
        self._test_encode(' \n \n', '=20\n=20\n')

    def test_encode_one_word_trailing_spaces(self):
        self._test_encode('hello   ', 'hello  =20')

    def test_encode_one_line_trailing_spaces(self):
        self._test_encode('hello   \n', 'hello  =20\n')

    def test_encode_one_word_trailing_tab(self):
        self._test_encode('hello  \t', 'hello  =09')

    def test_encode_one_line_trailing_tab(self):
        self._test_encode('hello  \t\n', 'hello  =09\n')

    def test_encode_trailing_space_before_maxlinelen(self):
        self._test_encode('abcd \n1234', 'abcd =\n\n1234', maxlinelen=6)

    def test_encode_trailing_space_at_maxlinelen(self):
        self._test_encode('abcd \n1234', 'abcd=\n=20\n1234', maxlinelen=5)

    def test_encode_trailing_space_beyond_maxlinelen(self):
        self._test_encode('abcd \n1234', 'abc=\nd=20\n1234', maxlinelen=4)

    def test_encode_whitespace_lines(self):
        self._test_encode(' \n' * 5, '=20\n' * 5)

    def test_encode_quoted_equals(self):
        self._test_encode('a = b', 'a =3D b')

    def test_encode_one_long_string(self):
        self._test_encode('x' * 100, 'x' * 75 + '=\n' + 'x' * 25)

    def test_encode_one_long_line(self):
        self._test_encode('x' * 100 + '\n', 'x' * 75 + '=\n' + 'x' * 25 + '\n')

    def test_encode_one_very_long_line(self):
        self._test_encode('x' * 200 + '\n',
                2 * ('x' * 75 + '=\n') + 'x' * 50 + '\n')

    def test_encode_shortest_maxlinelen(self):
        self._test_encode('=' * 5, '=3D=\n' * 4 + '=3D', maxlinelen=4)

    def test_encode_maxlinelen_too_small(self):
        self.assertRaises(ValueError, self._test_encode, '', '', maxlinelen=3)

    def test_encode(self):
        eq = self.assertEqual
        eq(quoprimime.body_encode(''), '')
        eq(quoprimime.body_encode('hello'), 'hello')
        # Test the binary flag
        eq(quoprimime.body_encode('hello\r\nworld'), 'hello\nworld')
        # Test the maxlinelen arg
        eq(quoprimime.body_encode('xxxx ' * 20, maxlinelen=40), """\
xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx=
 xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxx=
x xxxx xxxx xxxx xxxx=20""")
        # Test the eol argument
        eq(quoprimime.body_encode('xxxx ' * 20, maxlinelen=40, eol='\r\n'),
           """\
xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx=\r
 xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxx=\r
x xxxx xxxx xxxx xxxx=20""")
        eq(quoprimime.body_encode("""\
one line

two line"""), """\
one line

two line""")



# Test the Charset class
class TestCharset(unittest.TestCase):
    def tearDown(self):
        from email import charset as CharsetModule
        try:
            del CharsetModule.CHARSETS['fake']
        except KeyError:
            pass

    def test_codec_encodeable(self):
        eq = self.assertEqual
        # Make sure us-ascii = no Unicode conversion
        c = Charset('us-ascii')
        eq(c.header_encode('Hello World!'), 'Hello World!')
        # Test 8-bit idempotency with us-ascii
        s = '\xa4\xa2\xa4\xa4\xa4\xa6\xa4\xa8\xa4\xaa'
        self.assertRaises(UnicodeError, c.header_encode, s)
        c = Charset('utf-8')
        eq(c.header_encode(s), '=?utf-8?b?wqTCosKkwqTCpMKmwqTCqMKkwqo=?=')

    def test_body_encode(self):
        eq = self.assertEqual
        # Try a charset with QP body encoding
        c = Charset('iso-8859-1')
        eq('hello w=F6rld', c.body_encode('hello w\xf6rld'))
        # Try a charset with Base64 body encoding
        c = Charset('utf-8')
        eq('aGVsbG8gd29ybGQ=\n', c.body_encode(b'hello world'))
        # Try a charset with None body encoding
        c = Charset('us-ascii')
        eq('hello world', c.body_encode('hello world'))
        # Try the convert argument, where input codec != output codec
        c = Charset('euc-jp')
        # With apologies to Tokio Kikuchi ;)
        # XXX FIXME
##         try:
##             eq('\x1b$B5FCO;~IW\x1b(B',
##                c.body_encode('\xb5\xc6\xc3\xcf\xbb\xfe\xc9\xd7'))
##             eq('\xb5\xc6\xc3\xcf\xbb\xfe\xc9\xd7',
##                c.body_encode('\xb5\xc6\xc3\xcf\xbb\xfe\xc9\xd7', False))
##         except LookupError:
##             # We probably don't have the Japanese codecs installed
##             pass
        # Testing SF bug #625509, which we have to fake, since there are no
        # built-in encodings where the header encoding is QP but the body
        # encoding is not.
        from email import charset as CharsetModule
        CharsetModule.add_charset('fake', CharsetModule.QP, None, 'utf-8')
        c = Charset('fake')
        eq('hello world', c.body_encode('hello world'))

    def test_unicode_charset_name(self):
        charset = Charset('us-ascii')
        self.assertEqual(str(charset), 'us-ascii')
        self.assertRaises(errors.CharsetError, Charset, 'asc\xffii')



# Test multilingual MIME headers.
class TestHeader(TestEmailBase):
    def test_simple(self):
        eq = self.ndiffAssertEqual
        h = Header('Hello World!')
        eq(h.encode(), 'Hello World!')
        h.append(' Goodbye World!')
        eq(h.encode(), 'Hello World!  Goodbye World!')

    def test_simple_surprise(self):
        eq = self.ndiffAssertEqual
        h = Header('Hello World!')
        eq(h.encode(), 'Hello World!')
        h.append('Goodbye World!')
        eq(h.encode(), 'Hello World! Goodbye World!')

    def test_header_needs_no_decoding(self):
        h = 'no decoding needed'
        self.assertEqual(decode_header(h), [(h, None)])

    def test_long(self):
        h = Header("I am the very model of a modern Major-General; I've information vegetable, animal, and mineral; I know the kings of England, and I quote the fights historical from Marathon to Waterloo, in order categorical; I'm very well acquainted, too, with matters mathematical; I understand equations, both the simple and quadratical; about binomial theorem I'm teeming with a lot o' news, with many cheerful facts about the square of the hypotenuse.",
                   maxlinelen=76)
        for l in h.encode(splitchars=' ').split('\n '):
            self.assertLessEqual(len(l), 76)

    def test_multilingual(self):
        eq = self.ndiffAssertEqual
        g = Charset("iso-8859-1")
        cz = Charset("iso-8859-2")
        utf8 = Charset("utf-8")
        g_head = (b'Die Mieter treten hier ein werden mit einem '
                  b'Foerderband komfortabel den Korridor entlang, '
                  b'an s\xfcdl\xfcndischen Wandgem\xe4lden vorbei, '
                  b'gegen die rotierenden Klingen bef\xf6rdert. ')
        cz_head = (b'Finan\xe8ni metropole se hroutily pod tlakem jejich '
                   b'd\xf9vtipu.. ')
        utf8_head = ('\u6b63\u78ba\u306b\u8a00\u3046\u3068\u7ffb\u8a33\u306f'
                     '\u3055\u308c\u3066\u3044\u307e\u305b\u3093\u3002\u4e00'
                     '\u90e8\u306f\u30c9\u30a4\u30c4\u8a9e\u3067\u3059\u304c'
                     '\u3001\u3042\u3068\u306f\u3067\u305f\u3089\u3081\u3067'
                     '\u3059\u3002\u5b9f\u969b\u306b\u306f\u300cWenn ist das '
                     'Nunstuck git und Slotermeyer? Ja! Beiherhund das Oder '
                     'die Flipperwaldt gersput.\u300d\u3068\u8a00\u3063\u3066'
                     '\u3044\u307e\u3059\u3002')
        h = Header(g_head, g)
        h.append(cz_head, cz)
        h.append(utf8_head, utf8)
        enc = h.encode(maxlinelen=76)
        eq(enc, """\
=?iso-8859-1?q?Die_Mieter_treten_hier_ein_werden_mit_einem_Foerderband_kom?=
 =?iso-8859-1?q?fortabel_den_Korridor_entlang=2C_an_s=FCdl=FCndischen_Wand?=
 =?iso-8859-1?q?gem=E4lden_vorbei=2C_gegen_die_rotierenden_Klingen_bef=F6r?=
 =?iso-8859-1?q?dert=2E_?= =?iso-8859-2?q?Finan=E8ni_metropole_se_hroutily?=
 =?iso-8859-2?q?_pod_tlakem_jejich_d=F9vtipu=2E=2E_?= =?utf-8?b?5q2j56K6?=
 =?utf-8?b?44Gr6KiA44GG44Go57+76Kiz44Gv44GV44KM44Gm44GE44G+44Gb44KT44CC?=
 =?utf-8?b?5LiA6YOo44Gv44OJ44Kk44OE6Kqe44Gn44GZ44GM44CB44GC44Go44Gv44Gn?=
 =?utf-8?b?44Gf44KJ44KB44Gn44GZ44CC5a6f6Zqb44Gr44Gv44CMV2VubiBpc3QgZGFz?=
 =?utf-8?b?IE51bnN0dWNrIGdpdCB1bmQgU2xvdGVybWV5ZXI/IEphISBCZWloZXJodW5k?=
 =?utf-8?b?IGRhcyBPZGVyIGRpZSBGbGlwcGVyd2FsZHQgZ2Vyc3B1dC7jgI3jgajoqIA=?=
 =?utf-8?b?44Gj44Gm44GE44G+44GZ44CC?=""")
        decoded = decode_header(enc)
        eq(len(decoded), 3)
        eq(decoded[0], (g_head, 'iso-8859-1'))
        eq(decoded[1], (cz_head, 'iso-8859-2'))
        eq(decoded[2], (utf8_head.encode('utf-8'), 'utf-8'))
        ustr = str(h)
        eq(ustr,
           (b'Die Mieter treten hier ein werden mit einem Foerderband '
            b'komfortabel den Korridor entlang, an s\xc3\xbcdl\xc3\xbcndischen '
            b'Wandgem\xc3\xa4lden vorbei, gegen die rotierenden Klingen '
            b'bef\xc3\xb6rdert. Finan\xc4\x8dni metropole se hroutily pod '
            b'tlakem jejich d\xc5\xafvtipu.. \xe6\xad\xa3\xe7\xa2\xba\xe3\x81'
            b'\xab\xe8\xa8\x80\xe3\x81\x86\xe3\x81\xa8\xe7\xbf\xbb\xe8\xa8\xb3'
            b'\xe3\x81\xaf\xe3\x81\x95\xe3\x82\x8c\xe3\x81\xa6\xe3\x81\x84\xe3'
            b'\x81\xbe\xe3\x81\x9b\xe3\x82\x93\xe3\x80\x82\xe4\xb8\x80\xe9\x83'
            b'\xa8\xe3\x81\xaf\xe3\x83\x89\xe3\x82\xa4\xe3\x83\x84\xe8\xaa\x9e'
            b'\xe3\x81\xa7\xe3\x81\x99\xe3\x81\x8c\xe3\x80\x81\xe3\x81\x82\xe3'
            b'\x81\xa8\xe3\x81\xaf\xe3\x81\xa7\xe3\x81\x9f\xe3\x82\x89\xe3\x82'
            b'\x81\xe3\x81\xa7\xe3\x81\x99\xe3\x80\x82\xe5\xae\x9f\xe9\x9a\x9b'
            b'\xe3\x81\xab\xe3\x81\xaf\xe3\x80\x8cWenn ist das Nunstuck git '
            b'und Slotermeyer? Ja! Beiherhund das Oder die Flipperwaldt '
            b'gersput.\xe3\x80\x8d\xe3\x81\xa8\xe8\xa8\x80\xe3\x81\xa3\xe3\x81'
            b'\xa6\xe3\x81\x84\xe3\x81\xbe\xe3\x81\x99\xe3\x80\x82'
            ).decode('utf-8'))
        # Test make_header()
        newh = make_header(decode_header(enc))
        eq(newh, h)

    def test_empty_header_encode(self):
        h = Header()
        self.assertEqual(h.encode(), '')

    def test_header_ctor_default_args(self):
        eq = self.ndiffAssertEqual
        h = Header()
        eq(h, '')
        h.append('foo', Charset('iso-8859-1'))
        eq(h, 'foo')

    def test_explicit_maxlinelen(self):
        eq = self.ndiffAssertEqual
        hstr = ('A very long line that must get split to something other '
                'than at the 76th character boundary to test the non-default '
                'behavior')
        h = Header(hstr)
        eq(h.encode(), '''\
A very long line that must get split to something other than at the 76th
 character boundary to test the non-default behavior''')
        eq(str(h), hstr)
        h = Header(hstr, header_name='Subject')
        eq(h.encode(), '''\
A very long line that must get split to something other than at the
 76th character boundary to test the non-default behavior''')
        eq(str(h), hstr)
        h = Header(hstr, maxlinelen=1024, header_name='Subject')
        eq(h.encode(), hstr)
        eq(str(h), hstr)

    def test_quopri_splittable(self):
        eq = self.ndiffAssertEqual
        h = Header(charset='iso-8859-1', maxlinelen=20)
        x = 'xxxx ' * 20
        h.append(x)
        s = h.encode()
        eq(s, """\
=?iso-8859-1?q?xxx?=
 =?iso-8859-1?q?x_?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?_x?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?x_?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?_x?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?x_?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?_x?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?x_?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?_x?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?x_?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?_x?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?x_?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?_x?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?x_?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?_x?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?x_?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?_x?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?x_?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?_x?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?x_?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?xx?=
 =?iso-8859-1?q?_?=""")
        eq(x, str(make_header(decode_header(s))))
        h = Header(charset='iso-8859-1', maxlinelen=40)
        h.append('xxxx ' * 20)
        s = h.encode()
        eq(s, """\
=?iso-8859-1?q?xxxx_xxxx_xxxx_xxxx_xxx?=
 =?iso-8859-1?q?x_xxxx_xxxx_xxxx_xxxx_?=
 =?iso-8859-1?q?xxxx_xxxx_xxxx_xxxx_xx?=
 =?iso-8859-1?q?xx_xxxx_xxxx_xxxx_xxxx?=
 =?iso-8859-1?q?_xxxx_xxxx_?=""")
        eq(x, str(make_header(decode_header(s))))

    def test_base64_splittable(self):
        eq = self.ndiffAssertEqual
        h = Header(charset='koi8-r', maxlinelen=20)
        x = 'xxxx ' * 20
        h.append(x)
        s = h.encode()
        eq(s, """\
=?koi8-r?b?eHh4?=
 =?koi8-r?b?eCB4?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?IHh4?=
 =?koi8-r?b?eHgg?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?eCB4?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?IHh4?=
 =?koi8-r?b?eHgg?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?eCB4?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?IHh4?=
 =?koi8-r?b?eHgg?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?eCB4?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?IHh4?=
 =?koi8-r?b?eHgg?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?eCB4?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?IHh4?=
 =?koi8-r?b?eHgg?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?eCB4?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?IHh4?=
 =?koi8-r?b?eHgg?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?eCB4?=
 =?koi8-r?b?eHh4?=
 =?koi8-r?b?IA==?=""")
        eq(x, str(make_header(decode_header(s))))
        h = Header(charset='koi8-r', maxlinelen=40)
        h.append(x)
        s = h.encode()
        eq(s, """\
=?koi8-r?b?eHh4eCB4eHh4IHh4eHggeHh4?=
 =?koi8-r?b?eCB4eHh4IHh4eHggeHh4eCB4?=
 =?koi8-r?b?eHh4IHh4eHggeHh4eCB4eHh4?=
 =?koi8-r?b?IHh4eHggeHh4eCB4eHh4IHh4?=
 =?koi8-r?b?eHggeHh4eCB4eHh4IHh4eHgg?=
 =?koi8-r?b?eHh4eCB4eHh4IA==?=""")
        eq(x, str(make_header(decode_header(s))))

    def test_us_ascii_header(self):
        eq = self.assertEqual
        s = 'hello'
        x = decode_header(s)
        eq(x, [('hello', None)])
        h = make_header(x)
        eq(s, h.encode())

    def test_string_charset(self):
        eq = self.assertEqual
        h = Header()
        h.append('hello', 'iso-8859-1')
        eq(h, 'hello')

##    def test_unicode_error(self):
##        raises = self.assertRaises
##        raises(UnicodeError, Header, u'[P\xf6stal]', 'us-ascii')
##        raises(UnicodeError, Header, '[P\xf6stal]', 'us-ascii')
##        h = Header()
##        raises(UnicodeError, h.append, u'[P\xf6stal]', 'us-ascii')
##        raises(UnicodeError, h.append, '[P\xf6stal]', 'us-ascii')
##        raises(UnicodeError, Header, u'\u83ca\u5730\u6642\u592b', 'iso-8859-1')

    def test_utf8_shortest(self):
        eq = self.assertEqual
        h = Header('p\xf6stal', 'utf-8')
        eq(h.encode(), '=?utf-8?q?p=C3=B6stal?=')
        h = Header('\u83ca\u5730\u6642\u592b', 'utf-8')
        eq(h.encode(), '=?utf-8?b?6I+K5Zyw5pmC5aSr?=')

    def test_bad_8bit_header(self):
        raises = self.assertRaises
        eq = self.assertEqual
        x = b'Ynwp4dUEbay Auction Semiar- No Charge \x96 Earn Big'
        raises(UnicodeError, Header, x)
        h = Header()
        raises(UnicodeError, h.append, x)
        e = x.decode('utf-8', 'replace')
        eq(str(Header(x, errors='replace')), e)
        h.append(x, errors='replace')
        eq(str(h), e)

    def test_escaped_8bit_header(self):
        x = b'Ynwp4dUEbay Auction Semiar- No Charge \x96 Earn Big'
        e = x.decode('ascii', 'surrogateescape')
        h = Header(e, charset=email.charset.UNKNOWN8BIT)
        self.assertEqual(str(h),
                        'Ynwp4dUEbay Auction Semiar- No Charge \uFFFD Earn Big')
        self.assertEqual(email.header.decode_header(h), [(x, 'unknown-8bit')])

    def test_header_handles_binary_unknown8bit(self):
        x = b'Ynwp4dUEbay Auction Semiar- No Charge \x96 Earn Big'
        h = Header(x, charset=email.charset.UNKNOWN8BIT)
        self.assertEqual(str(h),
                        'Ynwp4dUEbay Auction Semiar- No Charge \uFFFD Earn Big')
        self.assertEqual(email.header.decode_header(h), [(x, 'unknown-8bit')])

    def test_make_header_handles_binary_unknown8bit(self):
        x = b'Ynwp4dUEbay Auction Semiar- No Charge \x96 Earn Big'
        h = Header(x, charset=email.charset.UNKNOWN8BIT)
        h2 = email.header.make_header(email.header.decode_header(h))
        self.assertEqual(str(h2),
                        'Ynwp4dUEbay Auction Semiar- No Charge \uFFFD Earn Big')
        self.assertEqual(email.header.decode_header(h2), [(x, 'unknown-8bit')])

    def test_modify_returned_list_does_not_change_header(self):
        h = Header('test')
        chunks = email.header.decode_header(h)
        chunks.append(('ascii', 'test2'))
        self.assertEqual(str(h), 'test')

    def test_encoded_adjacent_nonencoded(self):
        eq = self.assertEqual
        h = Header()
        h.append('hello', 'iso-8859-1')
        h.append('world')
        s = h.encode()
        eq(s, '=?iso-8859-1?q?hello?= world')
        h = make_header(decode_header(s))
        eq(h.encode(), s)

    def test_whitespace_keeper(self):
        eq = self.assertEqual
        s = 'Subject: =?koi8-r?b?8NLP18XSy8EgzsEgxsnOwczYztk=?= =?koi8-r?q?=CA?= zz.'
        parts = decode_header(s)
        eq(parts, [(b'Subject: ', None), (b'\xf0\xd2\xcf\xd7\xc5\xd2\xcb\xc1 \xce\xc1 \xc6\xc9\xce\xc1\xcc\xd8\xce\xd9\xca', 'koi8-r'), (b' zz.', None)])
        hdr = make_header(parts)
        eq(hdr.encode(),
           'Subject: =?koi8-r?b?8NLP18XSy8EgzsEgxsnOwczYztnK?= zz.')

    def test_broken_base64_header(self):
        raises = self.assertRaises
        s = 'Subject: =?EUC-KR?B?CSixpLDtKSC/7Liuvsax4iC6uLmwMcijIKHaILzSwd/H0SC8+LCjwLsgv7W/+Mj3I ?='
        raises(errors.HeaderParseError, decode_header, s)

    def test_shift_jis_charset(self):
        h = Header('文', charset='shift_jis')
        self.assertEqual(h.encode(), '=?iso-2022-jp?b?GyRCSjgbKEI=?=')

    def test_flatten_header_with_no_value(self):
        # Issue 11401 (regression from email 4.x)  Note that the space after
        # the header doesn't reflect the input, but this is also the way
        # email 4.x behaved.  At some point it would be nice to fix that.
        msg = email.message_from_string("EmptyHeader:")
        self.assertEqual(str(msg), "EmptyHeader: \n\n")

    def test_encode_preserves_leading_ws_on_value(self):
        msg = Message()
        msg['SomeHeader'] = '   value with leading ws'
        self.assertEqual(str(msg), "SomeHeader:    value with leading ws\n\n")

    def test_whitespace_header(self):
        self.assertEqual(Header(' ').encode(), ' ')



# Test RFC 2231 header parameters (en/de)coding
class TestRFC2231(TestEmailBase):

    # test_headerregistry.TestContentTypeHeader.rfc2231_encoded_with_double_quotes
    # test_headerregistry.TestContentTypeHeader.rfc2231_single_quote_inside_double_quotes
    def test_get_param(self):
        eq = self.assertEqual
        msg = self._msgobj('msg_29.txt')
        eq(msg.get_param('title'),
           ('us-ascii', 'en', 'This is even more ***fun*** isn\'t it!'))
        eq(msg.get_param('title', unquote=False),
           ('us-ascii', 'en', '"This is even more ***fun*** isn\'t it!"'))

    def test_set_param(self):
        eq = self.ndiffAssertEqual
        msg = Message()
        msg.set_param('title', 'This is even more ***fun*** isn\'t it!',
                      charset='us-ascii')
        eq(msg.get_param('title'),
           ('us-ascii', '', 'This is even more ***fun*** isn\'t it!'))
        msg.set_param('title', 'This is even more ***fun*** isn\'t it!',
                      charset='us-ascii', language='en')
        eq(msg.get_param('title'),
           ('us-ascii', 'en', 'This is even more ***fun*** isn\'t it!'))
        msg = self._msgobj('msg_01.txt')
        msg.set_param('title', 'This is even more ***fun*** isn\'t it!',
                      charset='us-ascii', language='en')
        eq(msg.as_string(maxheaderlen=78), """\
Return-Path: <bbb@zzz.org>
Delivered-To: bbb@zzz.org
Received: by mail.zzz.org (Postfix, from userid 889)
\tid 27CEAD38CC; Fri,  4 May 2001 14:05:44 -0400 (EDT)
MIME-Version: 1.0
Content-Transfer-Encoding: 7bit
Message-ID: <15090.61304.110929.45684@aaa.zzz.org>
From: bbb@ddd.com (John X. Doe)
To: bbb@zzz.org
Subject: This is a test message
Date: Fri, 4 May 2001 14:05:44 -0400
Content-Type: text/plain; charset=us-ascii;
 title*=us-ascii'en'This%20is%20even%20more%20%2A%2A%2Afun%2A%2A%2A%20isn%27t%20it%21


Hi,

Do you like this message?

-Me
""")

    def test_set_param_requote(self):
        msg = Message()
        msg.set_param('title', 'foo')
        self.assertEqual(msg['content-type'], 'text/plain; title="foo"')
        msg.set_param('title', 'bar', requote=False)
        self.assertEqual(msg['content-type'], 'text/plain; title=bar')
        # tspecial is still quoted.
        msg.set_param('title', "(bar)bell", requote=False)
        self.assertEqual(msg['content-type'], 'text/plain; title="(bar)bell"')

    def test_del_param(self):
        eq = self.ndiffAssertEqual
        msg = self._msgobj('msg_01.txt')
        msg.set_param('foo', 'bar', charset='us-ascii', language='en')
        msg.set_param('title', 'This is even more ***fun*** isn\'t it!',
            charset='us-ascii', language='en')
        msg.del_param('foo', header='Content-Type')
        eq(msg.as_string(maxheaderlen=78), """\
Return-Path: <bbb@zzz.org>
Delivered-To: bbb@zzz.org
Received: by mail.zzz.org (Postfix, from userid 889)
\tid 27CEAD38CC; Fri,  4 May 2001 14:05:44 -0400 (EDT)
MIME-Version: 1.0
Content-Transfer-Encoding: 7bit
Message-ID: <15090.61304.110929.45684@aaa.zzz.org>
From: bbb@ddd.com (John X. Doe)
To: bbb@zzz.org
Subject: This is a test message
Date: Fri, 4 May 2001 14:05:44 -0400
Content-Type: text/plain; charset="us-ascii";
 title*=us-ascii'en'This%20is%20even%20more%20%2A%2A%2Afun%2A%2A%2A%20isn%27t%20it%21


Hi,

Do you like this message?

-Me
""")

    # test_headerregistry.TestContentTypeHeader.rfc2231_encoded_charset
    # I changed the charset name, though, because the one in the file isn't
    # a legal charset name.  Should add a test for an illegal charset.
    def test_rfc2231_get_content_charset(self):
        eq = self.assertEqual
        msg = self._msgobj('msg_32.txt')
        eq(msg.get_content_charset(), 'us-ascii')

    # test_headerregistry.TestContentTypeHeader.rfc2231_encoded_no_double_quotes
    def test_rfc2231_parse_rfc_quoting(self):
        m = textwrap.dedent('''\
            Content-Disposition: inline;
            \tfilename*0*=''This%20is%20even%20more%20;
            \tfilename*1*=%2A%2A%2Afun%2A%2A%2A%20;
            \tfilename*2="is it not.pdf"

            ''')
        msg = email.message_from_string(m)
        self.assertEqual(msg.get_filename(),
                         'This is even more ***fun*** is it not.pdf')
        self.assertEqual(m, msg.as_string())

    # test_headerregistry.TestContentTypeHeader.rfc2231_encoded_with_double_quotes
    def test_rfc2231_parse_extra_quoting(self):
        m = textwrap.dedent('''\
            Content-Disposition: inline;
            \tfilename*0*="''This%20is%20even%20more%20";
            \tfilename*1*="%2A%2A%2Afun%2A%2A%2A%20";
            \tfilename*2="is it not.pdf"

            ''')
        msg = email.message_from_string(m)
        self.assertEqual(msg.get_filename(),
                         'This is even more ***fun*** is it not.pdf')
        self.assertEqual(m, msg.as_string())

    # test_headerregistry.TestContentTypeHeader.rfc2231_no_language_or_charset
    # but new test uses *0* because otherwise lang/charset is not valid.
    # test_headerregistry.TestContentTypeHeader.rfc2231_segmented_normal_values
    def test_rfc2231_no_language_or_charset(self):
        m = '''\
Content-Transfer-Encoding: 8bit
Content-Disposition: inline; filename="file____C__DOCUMENTS_20AND_20SETTINGS_FABIEN_LOCAL_20SETTINGS_TEMP_nsmail.htm"
Content-Type: text/html; NAME*0=file____C__DOCUMENTS_20AND_20SETTINGS_FABIEN_LOCAL_20SETTINGS_TEM; NAME*1=P_nsmail.htm

'''
        msg = email.message_from_string(m)
        param = msg.get_param('NAME')
        self.assertNotIsInstance(param, tuple)
        self.assertEqual(
            param,
            'file____C__DOCUMENTS_20AND_20SETTINGS_FABIEN_LOCAL_20SETTINGS_TEMP_nsmail.htm')

    # test_headerregistry.TestContentTypeHeader.rfc2231_encoded_no_charset
    def test_rfc2231_no_language_or_charset_in_filename(self):
        m = '''\
Content-Disposition: inline;
\tfilename*0*="''This%20is%20even%20more%20";
\tfilename*1*="%2A%2A%2Afun%2A%2A%2A%20";
\tfilename*2="is it not.pdf"

'''
        msg = email.message_from_string(m)
        self.assertEqual(msg.get_filename(),
                         'This is even more ***fun*** is it not.pdf')

    # Duplicate of previous test?
    def test_rfc2231_no_language_or_charset_in_filename_encoded(self):
        m = '''\
Content-Disposition: inline;
\tfilename*0*="''This%20is%20even%20more%20";
\tfilename*1*="%2A%2A%2Afun%2A%2A%2A%20";
\tfilename*2="is it not.pdf"

'''
        msg = email.message_from_string(m)
        self.assertEqual(msg.get_filename(),
                         'This is even more ***fun*** is it not.pdf')

    # test_headerregistry.TestContentTypeHeader.rfc2231_partly_encoded,
    # but the test below is wrong (the first part should be decoded).
    def test_rfc2231_partly_encoded(self):
        m = '''\
Content-Disposition: inline;
\tfilename*0="''This%20is%20even%20more%20";
\tfilename*1*="%2A%2A%2Afun%2A%2A%2A%20";
\tfilename*2="is it not.pdf"

'''
        msg = email.message_from_string(m)
        self.assertEqual(
            msg.get_filename(),
            'This%20is%20even%20more%20***fun*** is it not.pdf')

    def test_rfc2231_partly_nonencoded(self):
        m = '''\
Content-Disposition: inline;
\tfilename*0="This%20is%20even%20more%20";
\tfilename*1="%2A%2A%2Afun%2A%2A%2A%20";
\tfilename*2="is it not.pdf"

'''
        msg = email.message_from_string(m)
        self.assertEqual(
            msg.get_filename(),
            'This%20is%20even%20more%20%2A%2A%2Afun%2A%2A%2A%20is it not.pdf')

    def test_rfc2231_no_language_or_charset_in_boundary(self):
        m = '''\
Content-Type: multipart/alternative;
\tboundary*0*="''This%20is%20even%20more%20";
\tboundary*1*="%2A%2A%2Afun%2A%2A%2A%20";
\tboundary*2="is it not.pdf"

'''
        msg = email.message_from_string(m)
        self.assertEqual(msg.get_boundary(),
                         'This is even more ***fun*** is it not.pdf')

    def test_rfc2231_no_language_or_charset_in_charset(self):
        # This is a nonsensical charset value, but tests the code anyway
        m = '''\
Content-Type: text/plain;
\tcharset*0*="This%20is%20even%20more%20";
\tcharset*1*="%2A%2A%2Afun%2A%2A%2A%20";
\tcharset*2="is it not.pdf"

'''
        msg = email.message_from_string(m)
        self.assertEqual(msg.get_content_charset(),
                         'this is even more ***fun*** is it not.pdf')

    # test_headerregistry.TestContentTypeHeader.rfc2231_unknown_charset_treated_as_ascii
    def test_rfc2231_bad_encoding_in_filename(self):
        m = '''\
Content-Disposition: inline;
\tfilename*0*="bogus'xx'This%20is%20even%20more%20";
\tfilename*1*="%2A%2A%2Afun%2A%2A%2A%20";
\tfilename*2="is it not.pdf"

'''
        msg = email.message_from_string(m)
        self.assertEqual(msg.get_filename(),
                         'This is even more ***fun*** is it not.pdf')

    def test_rfc2231_bad_encoding_in_charset(self):
        m = """\
Content-Type: text/plain; charset*=bogus''utf-8%E2%80%9D

"""
        msg = email.message_from_string(m)
        # This should return None because non-ascii characters in the charset
        # are not allowed.
        self.assertEqual(msg.get_content_charset(), None)

    def test_rfc2231_bad_character_in_charset(self):
        m = """\
Content-Type: text/plain; charset*=ascii''utf-8%E2%80%9D

"""
        msg = email.message_from_string(m)
        # This should return None because non-ascii characters in the charset
        # are not allowed.
        self.assertEqual(msg.get_content_charset(), None)

    def test_rfc2231_bad_character_in_filename(self):
        m = '''\
Content-Disposition: inline;
\tfilename*0*="ascii'xx'This%20is%20even%20more%20";
\tfilename*1*="%2A%2A%2Afun%2A%2A%2A%20";
\tfilename*2*="is it not.pdf%E2"

'''
        msg = email.message_from_string(m)
        self.assertEqual(msg.get_filename(),
                         'This is even more ***fun*** is it not.pdf\ufffd')

    def test_rfc2231_unknown_encoding(self):
        m = """\
Content-Transfer-Encoding: 8bit
Content-Disposition: inline; filename*=X-UNKNOWN''myfile.txt

"""
        msg = email.message_from_string(m)
        self.assertEqual(msg.get_filename(), 'myfile.txt')

    def test_rfc2231_bad_character_in_encoding(self):
        m = """\
Content-Transfer-Encoding: 8bit
Content-Disposition: inline; filename*=utf-8\udce2\udc80\udc9d''myfile.txt

"""
        msg = email.message_from_string(m)
        self.assertEqual(msg.get_filename(), 'myfile.txt')

    def test_rfc2231_single_tick_in_filename_extended(self):
        eq = self.assertEqual
        m = """\
Content-Type: application/x-foo;
\tname*0*=\"Frank's\"; name*1*=\" Document\"

"""
        msg = email.message_from_string(m)
        charset, language, s = msg.get_param('name')
        eq(charset, None)
        eq(language, None)
        eq(s, "Frank's Document")

    # test_headerregistry.TestContentTypeHeader.rfc2231_single_quote_inside_double_quotes
    def test_rfc2231_single_tick_in_filename(self):
        m = """\
Content-Type: application/x-foo; name*0=\"Frank's\"; name*1=\" Document\"

"""
        msg = email.message_from_string(m)
        param = msg.get_param('name')
        self.assertNotIsInstance(param, tuple)
        self.assertEqual(param, "Frank's Document")

    def test_rfc2231_missing_tick(self):
        m = '''\
Content-Disposition: inline;
\tfilename*0*="'This%20is%20broken";
'''
        msg = email.message_from_string(m)
        self.assertEqual(
            msg.get_filename(),
            "'This is broken")

    def test_rfc2231_missing_tick_with_encoded_non_ascii(self):
        m = '''\
Content-Disposition: inline;
\tfilename*0*="'This%20is%E2broken";
'''
        msg = email.message_from_string(m)
        self.assertEqual(
            msg.get_filename(),
            "'This is\ufffdbroken")

    # test_headerregistry.TestContentTypeHeader.rfc2231_single_quote_in_value_with_charset_and_lang
    def test_rfc2231_tick_attack_extended(self):
        eq = self.assertEqual
        m = """\
Content-Type: application/x-foo;
\tname*0*=\"us-ascii'en-us'Frank's\"; name*1*=\" Document\"

"""
        msg = email.message_from_string(m)
        charset, language, s = msg.get_param('name')
        eq(charset, 'us-ascii')
        eq(language, 'en-us')
        eq(s, "Frank's Document")

    # test_headerregistry.TestContentTypeHeader.rfc2231_single_quote_in_non_encoded_value
    def test_rfc2231_tick_attack(self):
        m = """\
Content-Type: application/x-foo;
\tname*0=\"us-ascii'en-us'Frank's\"; name*1=\" Document\"

"""
        msg = email.message_from_string(m)
        param = msg.get_param('name')
        self.assertNotIsInstance(param, tuple)
        self.assertEqual(param, "us-ascii'en-us'Frank's Document")

    # test_headerregistry.TestContentTypeHeader.rfc2231_single_quotes_inside_quotes
    def test_rfc2231_no_extended_values(self):
        eq = self.assertEqual
        m = """\
Content-Type: application/x-foo; name=\"Frank's Document\"

"""
        msg = email.message_from_string(m)
        eq(msg.get_param('name'), "Frank's Document")

    # test_headerregistry.TestContentTypeHeader.rfc2231_encoded_then_unencoded_segments
    def test_rfc2231_encoded_then_unencoded_segments(self):
        eq = self.assertEqual
        m = """\
Content-Type: application/x-foo;
\tname*0*=\"us-ascii'en-us'My\";
\tname*1=\" Document\";
\tname*2*=\" For You\"

"""
        msg = email.message_from_string(m)
        charset, language, s = msg.get_param('name')
        eq(charset, 'us-ascii')
        eq(language, 'en-us')
        eq(s, 'My Document For You')

    # test_headerregistry.TestContentTypeHeader.rfc2231_unencoded_then_encoded_segments
    # test_headerregistry.TestContentTypeHeader.rfc2231_quoted_unencoded_then_encoded_segments
    def test_rfc2231_unencoded_then_encoded_segments(self):
        eq = self.assertEqual
        m = """\
Content-Type: application/x-foo;
\tname*0=\"us-ascii'en-us'My\";
\tname*1*=\" Document\";
\tname*2*=\" For You\"

"""
        msg = email.message_from_string(m)
        charset, language, s = msg.get_param('name')
        eq(charset, 'us-ascii')
        eq(language, 'en-us')
        eq(s, 'My Document For You')

    def test_should_not_hang_on_invalid_ew_messages(self):
        messages = ["""From: user@host.com
To: user@host.com
Bad-Header:
 =?us-ascii?Q?LCSwrV11+IB0rSbSker+M9vWR7wEDSuGqmHD89Gt=ea0nJFSaiz4vX3XMJPT4vrE?=
 =?us-ascii?Q?xGUZeOnp0o22pLBB7CYLH74Js=wOlK6Tfru2U47qR?=
 =?us-ascii?Q?72OfyEY2p2=2FrA9xNFyvH+fBTCmazxwzF8nGkK6D?=

Hello!
""", """From: ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ <xxx@xxx>
To: "xxx" <xxx@xxx>
Subject:   ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½
MIME-Version: 1.0
Content-Type: text/plain; charset="windows-1251";
Content-Transfer-Encoding: 8bit

ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
"""]
        for m in messages:
            with self.subTest(m=m):
                msg = email.message_from_string(m)


# Tests to ensure that signed parts of an email are completely preserved, as
# required by RFC1847 section 2.1.  Note that these are incomplete, because the
# email package does not currently always preserve the body.  See issue 1670765.
class TestSigned(TestEmailBase):

    def _msg_and_obj(self, filename):
        with openfile(filename, encoding="utf-8") as fp:
            original = fp.read()
            msg = email.message_from_string(original)
        return original, msg

    def _signed_parts_eq(self, original, result):
        # Extract the first mime part of each message
        import re
        repart = re.compile(r'^--([^\n]+)\n(.*?)\n--\1$', re.S | re.M)
        inpart = repart.search(original).group(2)
        outpart = repart.search(result).group(2)
        self.assertEqual(outpart, inpart)

    def test_long_headers_as_string(self):
        original, msg = self._msg_and_obj('msg_45.txt')
        result = msg.as_string()
        self._signed_parts_eq(original, result)

    def test_long_headers_as_string_maxheaderlen(self):
        original, msg = self._msg_and_obj('msg_45.txt')
        result = msg.as_string(maxheaderlen=60)
        self._signed_parts_eq(original, result)

    def test_long_headers_flatten(self):
        original, msg = self._msg_and_obj('msg_45.txt')
        fp = StringIO()
        Generator(fp).flatten(msg)
        result = fp.getvalue()
        self._signed_parts_eq(original, result)

class TestHeaderRegistry(TestEmailBase):
    # See issue gh-93010.
    def test_HeaderRegistry(self):
        reg = HeaderRegistry()
        a = reg('Content-Disposition', 'attachment; 0*00="foo"')
        self.assertIsInstance(a.defects[0], errors.InvalidHeaderDefect)

if __name__ == '__main__':
    unittest.main()