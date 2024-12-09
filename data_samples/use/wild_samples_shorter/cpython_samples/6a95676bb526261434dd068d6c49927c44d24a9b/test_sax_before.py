from io import BytesIO, StringIO
import codecs
import os.path
import shutil
import sys
from urllib.error import URLError
import urllib.request

        self.assertEqual(result.getvalue(), start + b"<doc>text</doc>")

    # ===== Locator support

    def test_expat_locator_noinfo(self):
        result = BytesIO()