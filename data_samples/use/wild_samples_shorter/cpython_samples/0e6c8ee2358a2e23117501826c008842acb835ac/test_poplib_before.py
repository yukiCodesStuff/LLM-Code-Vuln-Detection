    def test_rpop(self):
        self.assertOK(self.client.rpop('foo'))

    def test_apop(self):
        self.assertOK(self.client.apop('foo', 'dummypassword'))

    def test_top(self):
        expected =  (b'+OK 116 bytes',
                     [b'From: postmaster@python.org', b'Content-Type: text/plain',
                      b'MIME-Version: 1.0', b'Subject: Dummy', b'',