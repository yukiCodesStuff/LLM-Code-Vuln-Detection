    def testFailingHELO(self):
        self.assertRaises(smtplib.SMTPConnectError, smtplib.SMTP,
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

sim_auth = ('Mr.A@somewhere.com', 'somepassword')
sim_cram_md5_challenge = ('PENCeUxFREJoU0NnbmhNWitOMjNGNn'
                          'dAZWx3b29kLmlubm9zb2Z0LmNvbT4=')
sim_auth_credentials = {
    'login': 'TXIuQUBzb21ld2hlcmUuY29t',
    'plain': 'AE1yLkFAc29tZXdoZXJlLmNvbQBzb21lcGFzc3dvcmQ=',
    'cram-md5': ('TXIUQUBZB21LD2HLCMUUY29TIDG4OWQ0MJ'
                 'KWZGQ4ODNMNDA4NTGXMDRLZWMYZJDMODG1'),
    }
sim_auth_login_password = 'C29TZXBHC3N3B3JK'

sim_lists = {'list-1':['Mr.A@somewhere.com','Mrs.C@somewhereesle.com'],
             'list-2':['Ms.B@xn--fo-fka.com',],
            }

# Simulated SMTP channel & server
class SimSMTPChannel(smtpd.SMTPChannel):

    quit_response = None
    mail_response = None
    rcpt_response = None
    data_response = None
    rcpt_count = 0
    rset_count = 0

    def __init__(self, extra_features, *args, **kw):
        self._extrafeatures = ''.join(
            [ "250-{0}\r\n".format(x) for x in extra_features ])
        super(SimSMTPChannel, self).__init__(*args, **kw)

    def smtp_EHLO(self, arg):
        resp = ('250-testhost\r\n'
                '250-EXPN\r\n'
                '250-SIZE 20000000\r\n'
                '250-STARTTLS\r\n'
                '250-DELIVERBY\r\n')
        resp = resp + self._extrafeatures + '250 HELP'
        self.push(resp)
        self.seen_greeting = arg
        self.extended_smtp = True

    def smtp_VRFY(self, arg):
        # For max compatibility smtplib should be sending the raw address.
        if arg in sim_users:
            self.push('250 %s %s' % (sim_users[arg], smtplib.quoteaddr(arg)))
        else:
            self.push('550 No such user: %s' % arg)

    def smtp_EXPN(self, arg):
        list_name = arg.lower()
        if list_name in sim_lists:
            user_list = sim_lists[list_name]
            for n, user_email in enumerate(user_list):
                quoted_addr = smtplib.quoteaddr(user_email)
                if n < len(user_list) - 1:
                    self.push('250-%s %s' % (sim_users[user_email], quoted_addr))
                else:
                    self.push('250 %s %s' % (sim_users[user_email], quoted_addr))
        else:
            self.push('550 No access for you!')

    def smtp_AUTH(self, arg):
        if arg.strip().lower()=='cram-md5':
            self.push('334 {}'.format(sim_cram_md5_challenge))
            return
        mech, auth = arg.split()
        mech = mech.lower()
        if mech not in sim_auth_credentials:
            self.push('504 auth type unimplemented')
            return
        if mech == 'plain' and auth==sim_auth_credentials['plain']:
            self.push('235 plain auth ok')
        elif mech=='login' and auth==sim_auth_credentials['login']:
            self.push('334 Password:')
        else:
            self.push('550 No access for you!')

    def smtp_QUIT(self, arg):
        if self.quit_response is None:
            super(SimSMTPChannel, self).smtp_QUIT(arg)
        else:
            self.push(self.quit_response)
            self.close_when_done()

    def smtp_MAIL(self, arg):
        if self.mail_response is None:
            super().smtp_MAIL(arg)
        else:
            self.push(self.mail_response)

    def smtp_RCPT(self, arg):
        if self.rcpt_response is None:
            super().smtp_RCPT(arg)
            return
        self.rcpt_count += 1
        self.push(self.rcpt_response[self.rcpt_count-1])

    def smtp_RSET(self, arg):
        self.rset_count += 1
        super().smtp_RSET(arg)

    def smtp_DATA(self, arg):
        if self.data_response is None:
            super().smtp_DATA(arg)
        else:
            self.push(self.data_response)

    def handle_error(self):
        raise


class SimSMTPServer(smtpd.SMTPServer):

    channel_class = SimSMTPChannel

    def __init__(self, *args, **kw):
        self._extra_features = []
        smtpd.SMTPServer.__init__(self, *args, **kw)

    def handle_accepted(self, conn, addr):
        self._SMTPchannel = self.channel_class(
            self._extra_features, self, conn, addr)

    def process_message(self, peer, mailfrom, rcpttos, data):
        pass

    def add_feature(self, feature):
        self._extra_features.append(feature)

    def handle_error(self):
        raise


# Test various SMTP & ESMTP commands/behaviors that require a simulated server
# (i.e., something with more features than DebuggingServer)
@unittest.skipUnless(threading, 'Threading required for this test.')
class SMTPSimTests(unittest.TestCase):

    def setUp(self):
        self.real_getfqdn = socket.getfqdn
        socket.getfqdn = mock_socket.getfqdn
        self.serv_evt = threading.Event()
        self.client_evt = threading.Event()
        # Pick a random unused port by passing 0 for the port number
        self.serv = SimSMTPServer((HOST, 0), ('nowhere', -1))
        # Keep a note of what port was assigned
        self.port = self.serv.socket.getsockname()[1]
        serv_args = (self.serv, self.serv_evt, self.client_evt)
        self.thread = threading.Thread(target=debugging_server, args=serv_args)
        self.thread.start()

        # wait until server thread has assigned a port number
        self.serv_evt.wait()
        self.serv_evt.clear()

    def tearDown(self):
        socket.getfqdn = self.real_getfqdn
        # indicate that the client is finished
        self.client_evt.set()
        # wait for the server thread to terminate
        self.serv_evt.wait()
        self.thread.join()

    def testBasic(self):
        # smoke test
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)
        smtp.quit()

    def testEHLO(self):
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)

        # no features should be present before the EHLO
        self.assertEqual(smtp.esmtp_features, {})

        # features expected from the test server
        expected_features = {'expn':'',
                             'size': '20000000',
                             'starttls': '',
                             'deliverby': '',
                             'help': '',
                             }

        smtp.ehlo()
        self.assertEqual(smtp.esmtp_features, expected_features)
        for k in expected_features:
            self.assertTrue(smtp.has_extn(k))
        self.assertFalse(smtp.has_extn('unsupported-feature'))
        smtp.quit()

    def testVRFY(self):
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)

        for email, name in sim_users.items():
            expected_known = (250, bytes('%s %s' %
                                         (name, smtplib.quoteaddr(email)),
                                         "ascii"))
            self.assertEqual(smtp.vrfy(email), expected_known)

        u = 'nobody@nowhere.com'
        expected_unknown = (550, ('No such user: %s' % u).encode('ascii'))
        self.assertEqual(smtp.vrfy(u), expected_unknown)
        smtp.quit()

    def testEXPN(self):
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)

        for listname, members in sim_lists.items():
            users = []
            for m in members:
                users.append('%s %s' % (sim_users[m], smtplib.quoteaddr(m)))
            expected_known = (250, bytes('\n'.join(users), "ascii"))
            self.assertEqual(smtp.expn(listname), expected_known)

        u = 'PSU-Members-List'
        expected_unknown = (550, b'No access for you!')
        self.assertEqual(smtp.expn(u), expected_unknown)
        smtp.quit()

    def testAUTH_PLAIN(self):
        self.serv.add_feature("AUTH PLAIN")
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)

        expected_auth_ok = (235, b'plain auth ok')
        self.assertEqual(smtp.login(sim_auth[0], sim_auth[1]), expected_auth_ok)
        smtp.close()

    # SimSMTPChannel doesn't fully support LOGIN or CRAM-MD5 auth because they
    # require a synchronous read to obtain the credentials...so instead smtpd
    # sees the credential sent by smtplib's login method as an unknown command,
    # which results in smtplib raising an auth error.  Fortunately the error
    # message contains the encoded credential, so we can partially check that it
    # was generated correctly (partially, because the 'word' is uppercased in
    # the error message).

    def testAUTH_LOGIN(self):
        self.serv.add_feature("AUTH LOGIN")
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)
        try: smtp.login(sim_auth[0], sim_auth[1])
        except smtplib.SMTPAuthenticationError as err:
            self.assertIn(sim_auth_login_password, str(err))
        smtp.close()

    def testAUTH_CRAM_MD5(self):
        self.serv.add_feature("AUTH CRAM-MD5")
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)

        try: smtp.login(sim_auth[0], sim_auth[1])
        except smtplib.SMTPAuthenticationError as err:
            self.assertIn(sim_auth_credentials['cram-md5'], str(err))
        smtp.close()

    def test_with_statement(self):
        with smtplib.SMTP(HOST, self.port) as smtp:
            code, message = smtp.noop()
            self.assertEqual(code, 250)
        self.assertRaises(smtplib.SMTPServerDisconnected, smtp.send, b'foo')
        with smtplib.SMTP(HOST, self.port) as smtp:
            smtp.close()
        self.assertRaises(smtplib.SMTPServerDisconnected, smtp.send, b'foo')

    def test_with_statement_QUIT_failure(self):
        with self.assertRaises(smtplib.SMTPResponseException) as error:
            with smtplib.SMTP(HOST, self.port) as smtp:
                smtp.noop()
                self.serv._SMTPchannel.quit_response = '421 QUIT FAILED'
        self.assertEqual(error.exception.smtp_code, 421)
        self.assertEqual(error.exception.smtp_error, b'QUIT FAILED')

    #TODO: add tests for correct AUTH method fallback now that the
    #test infrastructure can support it.

    # Issue 5713: make sure close, not rset, is called if we get a 421 error
    def test_421_from_mail_cmd(self):
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)
        smtp.noop()
        self.serv._SMTPchannel.mail_response = '421 closing connection'
        with self.assertRaises(smtplib.SMTPSenderRefused):
            smtp.sendmail('John', 'Sally', 'test message')
        self.assertIsNone(smtp.sock)
        self.assertEqual(self.serv._SMTPchannel.rset_count, 0)

    def test_421_from_rcpt_cmd(self):
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)
        smtp.noop()
        self.serv._SMTPchannel.rcpt_response = ['250 accepted', '421 closing']
        with self.assertRaises(smtplib.SMTPRecipientsRefused) as r:
            smtp.sendmail('John', ['Sally', 'Frank', 'George'], 'test message')
        self.assertIsNone(smtp.sock)
        self.assertEqual(self.serv._SMTPchannel.rset_count, 0)
        self.assertDictEqual(r.exception.args[0], {'Frank': (421, b'closing')})

    def test_421_from_data_cmd(self):
        class MySimSMTPChannel(SimSMTPChannel):
            def found_terminator(self):
                if self.smtp_state == self.DATA:
                    self.push('421 closing')
                else:
                    super().found_terminator()
        self.serv.channel_class = MySimSMTPChannel
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)
        smtp.noop()
        with self.assertRaises(smtplib.SMTPDataError):
            smtp.sendmail('John@foo.org', ['Sally@foo.org'], 'test message')
        self.assertIsNone(smtp.sock)
        self.assertEqual(self.serv._SMTPchannel.rcpt_count, 0)


@support.reap_threads
def test_main(verbose=None):
    support.run_unittest(GeneralTests, DebuggingServerTests,
                              NonConnectingTests,
                              BadHELOServerTests, SMTPSimTests,
                              TooLongLineTests)

if __name__ == '__main__':
    test_main()
def test_main(verbose=None):
    support.run_unittest(GeneralTests, DebuggingServerTests,
                              NonConnectingTests,
                              BadHELOServerTests, SMTPSimTests,
                              TooLongLineTests)

if __name__ == '__main__':
    test_main()