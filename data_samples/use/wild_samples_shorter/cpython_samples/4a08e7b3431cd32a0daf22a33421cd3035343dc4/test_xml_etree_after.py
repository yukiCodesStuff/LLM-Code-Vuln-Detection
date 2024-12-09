import operator
import os
import pickle
import pyexpat
import sys
import textwrap
import types
import unittest
</foo>
"""

fails_with_expat_2_6_0 = (unittest.expectedFailure
                        if pyexpat.version_info >= (2, 6, 0) else
                        lambda test: test)

def checkwarnings(*filters, quiet=False):
    def decorator(test):
        def newtest(*args, **kwargs):
            with warnings_helper.check_warnings(*filters, quiet=quiet):
        self.assertEqual([(action, elem.tag) for action, elem in events],
                         expected)

    def test_simple_xml(self, chunk_size=None):
        parser = ET.XMLPullParser()
        self.assert_event_tags(parser, [])
        self._feed(parser, "<!-- comment -->\n", chunk_size)
        self.assert_event_tags(parser, [])
        self._feed(parser,
                   "<root>\n  <element key='value'>text</element",
                   chunk_size)
        self.assert_event_tags(parser, [])
        self._feed(parser, ">\n", chunk_size)
        self.assert_event_tags(parser, [('end', 'element')])
        self._feed(parser, "<element>text</element>tail\n", chunk_size)
        self._feed(parser, "<empty-element/>\n", chunk_size)
        self.assert_event_tags(parser, [
            ('end', 'element'),
            ('end', 'empty-element'),
            ])
        self._feed(parser, "</root>\n", chunk_size)
        self.assert_event_tags(parser, [('end', 'root')])
        self.assertIsNone(parser.close())

    @fails_with_expat_2_6_0
    def test_simple_xml_chunk_1(self):
        self.test_simple_xml(chunk_size=1)

    @fails_with_expat_2_6_0
    def test_simple_xml_chunk_5(self):
        self.test_simple_xml(chunk_size=5)

    def test_simple_xml_chunk_22(self):
        self.test_simple_xml(chunk_size=22)

    def test_feed_while_iterating(self):
        parser = ET.XMLPullParser()
        it = parser.read_events()