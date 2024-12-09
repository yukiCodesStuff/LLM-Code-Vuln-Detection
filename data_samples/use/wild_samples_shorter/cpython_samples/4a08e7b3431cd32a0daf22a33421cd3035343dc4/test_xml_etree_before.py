import operator
import os
import pickle
import sys
import textwrap
import types
import unittest
</foo>
"""

def checkwarnings(*filters, quiet=False):
    def decorator(test):
        def newtest(*args, **kwargs):
            with warnings_helper.check_warnings(*filters, quiet=quiet):
        self.assertEqual([(action, elem.tag) for action, elem in events],
                         expected)

    def test_simple_xml(self):
        for chunk_size in (None, 1, 5):
            with self.subTest(chunk_size=chunk_size):
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

    def test_feed_while_iterating(self):
        parser = ET.XMLPullParser()
        it = parser.read_events()