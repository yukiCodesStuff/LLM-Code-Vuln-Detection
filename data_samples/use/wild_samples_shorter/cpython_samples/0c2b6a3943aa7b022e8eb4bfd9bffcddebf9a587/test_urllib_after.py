        filename, _ = urllib.request.URLopener().retrieve(url)
        self.assertEqual(os.path.splitext(filename)[1], ".txt")

    @support.ignore_warnings(category=DeprecationWarning)
    def test_local_file_open(self):
        # bpo-35907, CVE-2019-9948: urllib must reject local_file:// scheme
        class DummyURLopener(urllib.request.URLopener):
            def open_local_file(self, url):
                return url
        for url in ('local_file://example', 'local-file://example'):
            self.assertRaises(OSError, urllib.request.urlopen, url)
            self.assertRaises(OSError, urllib.request.URLopener().open, url)
            self.assertRaises(OSError, urllib.request.URLopener().retrieve, url)
            self.assertRaises(OSError, DummyURLopener().open, url)
            self.assertRaises(OSError, DummyURLopener().retrieve, url)


# Just commented them out.
# Can't really tell why keep failing in windows and sparc.
# Everywhere else they work ok, but on those machines, sometimes