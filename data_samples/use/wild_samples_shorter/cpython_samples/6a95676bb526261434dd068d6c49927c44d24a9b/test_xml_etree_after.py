</foo>
"""

def checkwarnings(*filters, quiet=False):
    def decorator(test):
        def newtest(*args, **kwargs):
            with warnings_helper.check_warnings(*filters, quiet=quiet):

class XMLPullParserTest(unittest.TestCase):

    def _feed(self, parser, data, chunk_size=None, flush=False):
        if chunk_size is None:
            parser.feed(data)
        else:
            for i in range(0, len(data), chunk_size):
                parser.feed(data[i:i+chunk_size])
        if flush:
            parser.flush()

    def assert_events(self, parser, expected, max_events=None):
        self.assertEqual(
            [(event, (elem.tag, elem.text))
        self.assertEqual([(action, elem.tag) for action, elem in events],
                         expected)

    def test_simple_xml(self, chunk_size=None, flush=False):
        parser = ET.XMLPullParser()
        self.assert_event_tags(parser, [])
        self._feed(parser, "<!-- comment -->\n", chunk_size, flush)
        self.assert_event_tags(parser, [])
        self._feed(parser,
                   "<root>\n  <element key='value'>text</element",
                   chunk_size, flush)
        self.assert_event_tags(parser, [])
        self._feed(parser, ">\n", chunk_size, flush)
        self.assert_event_tags(parser, [('end', 'element')])
        self._feed(parser, "<element>text</element>tail\n", chunk_size, flush)
        self._feed(parser, "<empty-element/>\n", chunk_size, flush)
        self.assert_event_tags(parser, [
            ('end', 'element'),
            ('end', 'empty-element'),
            ])
        self._feed(parser, "</root>\n", chunk_size, flush)
        self.assert_event_tags(parser, [('end', 'root')])
        self.assertIsNone(parser.close())

    def test_simple_xml_chunk_1(self):
        self.test_simple_xml(chunk_size=1, flush=True)

    def test_simple_xml_chunk_5(self):
        self.test_simple_xml(chunk_size=5, flush=True)

    def test_simple_xml_chunk_22(self):
        self.test_simple_xml(chunk_size=22)

        with self.assertRaises(ValueError):
            ET.XMLPullParser(events=('start', 'end', 'bogus'))

    def test_flush_reparse_deferral_enabled(self):
        if pyexpat.version_info < (2, 6, 0):
            self.skipTest(f'Expat {pyexpat.version_info} does not '
                          'support reparse deferral')

        parser = ET.XMLPullParser(events=('start', 'end'))

        for chunk in ("<doc", ">"):
            parser.feed(chunk)

        self.assert_event_tags(parser, [])  # i.e. no elements started
        if ET is pyET:
            self.assertTrue(parser._parser._parser.GetReparseDeferralEnabled())

        parser.flush()

        self.assert_event_tags(parser, [('start', 'doc')])
        if ET is pyET:
            self.assertTrue(parser._parser._parser.GetReparseDeferralEnabled())

        parser.feed("</doc>")
        parser.close()

        self.assert_event_tags(parser, [('end', 'doc')])

    def test_flush_reparse_deferral_disabled(self):
        parser = ET.XMLPullParser(events=('start', 'end'))

        for chunk in ("<doc", ">"):
            parser.feed(chunk)

        if pyexpat.version_info >= (2, 6, 0):
            if not ET is pyET:
                self.skipTest(f'XMLParser.(Get|Set)ReparseDeferralEnabled '
                              'methods not available in C')
            parser._parser._parser.SetReparseDeferralEnabled(False)

        self.assert_event_tags(parser, [])  # i.e. no elements started
        if ET is pyET:
            self.assertFalse(parser._parser._parser.GetReparseDeferralEnabled())

        parser.flush()

        self.assert_event_tags(parser, [('start', 'doc')])
        if ET is pyET:
            self.assertFalse(parser._parser._parser.GetReparseDeferralEnabled())

        parser.feed("</doc>")
        parser.close()

        self.assert_event_tags(parser, [('end', 'doc')])

#
# xinclude tests (samples from appendix C of the xinclude specification)
