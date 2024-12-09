    def test_urlopener_retrieve_remote(self):
        url = "http://www.python.org/file.txt"
        self.fakehttp(b"HTTP/1.1 200 OK\r\n\r\nHello!")
        self.addCleanup(self.unfakehttp)
        filename, _ = urllib.request.URLopener().retrieve(url)
        self.assertEqual(os.path.splitext(filename)[1], ".txt")


# Just commented them out.
# Can't really tell why keep failing in windows and sparc.
# Everywhere else they work ok, but on those machines, sometimes
# fail in one of the tests, sometimes in other. I have a linux, and
# the tests go ok.
# If anybody has one of the problematic environments, please help!
# .   Facundo
#
# def server(evt):
#     import socket, time
#     serv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#     serv.settimeout(3)
#     serv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
#     serv.bind(("", 9093))
#     serv.listen()
#     try:
#         conn, addr = serv.accept()
#         conn.send("1 Hola mundo\n")
#         cantdata = 0
#         while cantdata < 13:
#             data = conn.recv(13-cantdata)
#             cantdata += len(data)
#             time.sleep(.3)
#         conn.send("2 No more lines\n")
#         conn.close()
#     except socket.timeout:
#         pass
#     finally:
#         serv.close()
#         evt.set()
#
# class FTPWrapperTests(unittest.TestCase):
#
#     def setUp(self):
#         import ftplib, time, threading
#         ftplib.FTP.port = 9093
#         self.evt = threading.Event()
#         threading.Thread(target=server, args=(self.evt,)).start()
#         time.sleep(.1)
#
#     def tearDown(self):
#         self.evt.wait()
#
#     def testBasic(self):
#         # connects
#         ftp = urllib.ftpwrapper("myuser", "mypass", "localhost", 9093, [])
#         ftp.close()
#
#     def testTimeoutNone(self):
#         # global default timeout is ignored
#         import socket
#         self.assertIsNone(socket.getdefaulttimeout())
#         socket.setdefaulttimeout(30)
#         try:
#             ftp = urllib.ftpwrapper("myuser", "mypass", "localhost", 9093, [])
#         finally:
#             socket.setdefaulttimeout(None)
#         self.assertEqual(ftp.ftp.sock.gettimeout(), 30)
#         ftp.close()
#
#     def testTimeoutDefault(self):
#         # global default timeout is used
#         import socket
#         self.assertIsNone(socket.getdefaulttimeout())
#         socket.setdefaulttimeout(30)
#         try:
#             ftp = urllib.ftpwrapper("myuser", "mypass", "localhost", 9093, [])
#         finally:
#             socket.setdefaulttimeout(None)
#         self.assertEqual(ftp.ftp.sock.gettimeout(), 30)
#         ftp.close()
#
#     def testTimeoutValue(self):
#         ftp = urllib.ftpwrapper("myuser", "mypass", "localhost", 9093, [],
#                                 timeout=30)
#         self.assertEqual(ftp.ftp.sock.gettimeout(), 30)
#         ftp.close()


class RequestTests(unittest.TestCase):
    """Unit tests for urllib.request.Request."""

    def test_default_values(self):
        Request = urllib.request.Request
        request = Request("http://www.python.org")
        self.assertEqual(request.get_method(), 'GET')
        request = Request("http://www.python.org", {})
        self.assertEqual(request.get_method(), 'POST')

    def test_with_method_arg(self):
        Request = urllib.request.Request
        request = Request("http://www.python.org", method='HEAD')
        self.assertEqual(request.method, 'HEAD')
        self.assertEqual(request.get_method(), 'HEAD')
        request = Request("http://www.python.org", {}, method='HEAD')
        self.assertEqual(request.method, 'HEAD')
        self.assertEqual(request.get_method(), 'HEAD')
        request = Request("http://www.python.org", method='GET')
        self.assertEqual(request.get_method(), 'GET')
        request.method = 'HEAD'
        self.assertEqual(request.get_method(), 'HEAD')


class URL2PathNameTests(unittest.TestCase):

    def test_converting_drive_letter(self):
        self.assertEqual(url2pathname("///C|"), 'C:')
        self.assertEqual(url2pathname("///C:"), 'C:')
        self.assertEqual(url2pathname("///C|/"), 'C:\\')

    def test_converting_when_no_drive_letter(self):
        # cannot end a raw string in \
        self.assertEqual(url2pathname("///C/test/"), r'\\\C\test' '\\')
        self.assertEqual(url2pathname("////C/test/"), r'\\C\test' '\\')

    def test_simple_compare(self):
        self.assertEqual(url2pathname("///C|/foo/bar/spam.foo"),
                         r'C:\foo\bar\spam.foo')

    def test_non_ascii_drive_letter(self):
        self.assertRaises(IOError, url2pathname, "///\u00e8|/")

    def test_roundtrip_url2pathname(self):
        list_of_paths = ['C:',
                         r'\\\C\test\\',
                         r'C:\foo\bar\spam.foo'
                         ]
        for path in list_of_paths:
            self.assertEqual(url2pathname(pathname2url(path)), path)

class PathName2URLTests(unittest.TestCase):

    def test_converting_drive_letter(self):
        self.assertEqual(pathname2url("C:"), '///C:')
        self.assertEqual(pathname2url("C:\\"), '///C:')

    def test_converting_when_no_drive_letter(self):
        self.assertEqual(pathname2url(r"\\\folder\test" "\\"),
                         '/////folder/test/')
        self.assertEqual(pathname2url(r"\\folder\test" "\\"),
                         '////folder/test/')
        self.assertEqual(pathname2url(r"\folder\test" "\\"),
                         '/folder/test/')

    def test_simple_compare(self):
        self.assertEqual(pathname2url(r'C:\foo\bar\spam.foo'),
                         "///C:/foo/bar/spam.foo" )

    def test_long_drive_letter(self):
        self.assertRaises(IOError, pathname2url, "XX:\\")

    def test_roundtrip_pathname2url(self):
        list_of_paths = ['///C:',
                         '/////folder/test/',
                         '///C:/foo/bar/spam.foo']
        for path in list_of_paths:
            self.assertEqual(pathname2url(url2pathname(path)), path)

if __name__ == '__main__':
    unittest.main()