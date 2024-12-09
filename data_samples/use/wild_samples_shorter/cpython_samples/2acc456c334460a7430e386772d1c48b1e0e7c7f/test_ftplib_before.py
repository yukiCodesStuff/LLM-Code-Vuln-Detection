        self.next_response = ''
        self.next_data = None
        self.rest = None
        self.push('220 welcome')

    def collect_incoming_data(self, data):
        self.in_buffer.append(data)
            offset = int(self.rest)
        else:
            offset = 0
        self.dtp.push(RETR_DATA[offset:])
        self.dtp.close_when_done()
        self.rest = None

    def cmd_list(self, arg):
        self.dtp.push(MLSD_DATA)
        self.dtp.close_when_done()


class DummyFTPServer(asyncore.dispatcher, threading.Thread):

    handler = DummyFTPHandler
        self.assertEqual(ftplib.parse257('257 "/foo/b""ar"'), '/foo/b"ar')
        self.assertEqual(ftplib.parse257('257 "/foo/b""ar" created'), '/foo/b"ar')


class TestIPv6Environment(TestCase):

    def setUp(self):