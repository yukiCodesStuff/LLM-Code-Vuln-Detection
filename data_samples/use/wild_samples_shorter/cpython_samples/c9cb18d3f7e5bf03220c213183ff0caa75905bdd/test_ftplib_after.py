        self.last_received_data = ''
        self.next_response = ''
        self.rest = None
        self.next_retr_data = RETR_DATA
        self.push('220 welcome')

    def collect_incoming_data(self, data):
        self.in_buffer.append(data)
            offset = int(self.rest)
        else:
            offset = 0
        self.dtp.push(self.next_retr_data[offset:])
        self.dtp.close_when_done()
        self.rest = None

    def cmd_list(self, arg):
        self.dtp.push(NLST_DATA)
        self.dtp.close_when_done()

    def cmd_setlongretr(self, arg):
        # For testing. Next RETR will return long line.
        self.next_retr_data = 'x' * int(arg)
        self.push('125 setlongretr ok')


class DummyFTPServer(asyncore.dispatcher, threading.Thread):

    handler = DummyFTPHandler
        self.assertEqual(ftplib.parse257('257 "/foo/b""ar"'), '/foo/b"ar')
        self.assertEqual(ftplib.parse257('257 "/foo/b""ar" created'), '/foo/b"ar')

    def test_line_too_long(self):
        self.assertRaises(ftplib.Error, self.client.sendcmd,
                          'x' * self.client.maxline * 2)

    def test_retrlines_too_long(self):
        self.client.sendcmd('SETLONGRETR %d' % (self.client.maxline * 2))
        received = []
        self.assertRaises(ftplib.Error,
                          self.client.retrlines, 'retr', received.append)

    def test_storlines_too_long(self):
        f = io.BytesIO(b'x' * self.client.maxline * 2)
        self.assertRaises(ftplib.Error, self.client.storlines, 'stor', f)


class TestIPv6Environment(TestCase):

    def setUp(self):