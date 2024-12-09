                            HOST, self.port, 'localhost', 3)


@unittest.skipUnless(threading, 'Threading required for this test.')
class TooLongLineTests(unittest.TestCase):
    respdata = b'250 OK' + (b'.' * smtplib._MAXLINE * 2) + b'\n'

    def setUp(self):
        self.old_stdout = sys.stdout
        self.output = io.StringIO()
        sys.stdout = self.output

        self.evt = threading.Event()
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(15)
        self.port = support.bind_port(self.sock)
        servargs = (self.evt, self.respdata, self.sock)
        threading.Thread(target=server, args=servargs).start()
        self.evt.wait()
        self.evt.clear()

    def tearDown(self):
        self.evt.wait()
        sys.stdout = self.old_stdout

    def testLineTooLong(self):
        self.assertRaises(smtplib.SMTPResponseException, smtplib.SMTP,
                          HOST, self.port, 'localhost', 3)


sim_users = {'Mr.A@somewhere.com':'John A',
             'Ms.B@xn--fo-fka.com':'Sally B',
             'Mrs.C@somewhereesle.com':'Ruth C',
            }
def test_main(verbose=None):
    support.run_unittest(GeneralTests, DebuggingServerTests,
                              NonConnectingTests,
                              BadHELOServerTests, SMTPSimTests,
                              TooLongLineTests)

if __name__ == '__main__':
    test_main()