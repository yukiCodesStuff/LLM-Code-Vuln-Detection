from io import BytesIO, StringIO
import codecs
import os.path
import pyexpat
import shutil
import sys
from urllib.error import URLError
import urllib.request

        self.assertEqual(result.getvalue(), start + b"<doc>text</doc>")

    def test_flush_reparse_deferral_enabled(self):
        if pyexpat.version_info < (2, 6, 0):
            self.skipTest(f'Expat {pyexpat.version_info} does not support reparse deferral')

        result = BytesIO()
        xmlgen = XMLGenerator(result)
        parser = create_parser()
        parser.setContentHandler(xmlgen)

        for chunk in ("<doc", ">"):
            parser.feed(chunk)

        self.assertEqual(result.getvalue(), start)  # i.e. no elements started
        self.assertTrue(parser._parser.GetReparseDeferralEnabled())

        parser.flush()

        self.assertTrue(parser._parser.GetReparseDeferralEnabled())
        self.assertEqual(result.getvalue(), start + b"<doc>")

        parser.feed("</doc>")
        parser.close()

        self.assertEqual(result.getvalue(), start + b"<doc></doc>")

    def test_flush_reparse_deferral_disabled(self):
        result = BytesIO()
        xmlgen = XMLGenerator(result)
        parser = create_parser()
        parser.setContentHandler(xmlgen)

        for chunk in ("<doc", ">"):
            parser.feed(chunk)

        if pyexpat.version_info >= (2, 6, 0):
            parser._parser.SetReparseDeferralEnabled(False)

        self.assertEqual(result.getvalue(), start)  # i.e. no elements started
        self.assertFalse(parser._parser.GetReparseDeferralEnabled())

        parser.flush()

        self.assertFalse(parser._parser.GetReparseDeferralEnabled())
        self.assertEqual(result.getvalue(), start + b"<doc>")

        parser.feed("</doc>")
        parser.close()

        self.assertEqual(result.getvalue(), start + b"<doc></doc>")

    # ===== Locator support

    def test_expat_locator_noinfo(self):
        result = BytesIO()