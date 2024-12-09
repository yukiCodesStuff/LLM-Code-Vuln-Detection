        thread.join()
        self.assertEqual(result, b"proxied data\n")

    def test_putrequest_override_validation(self):
        """
        It should be possible to override the default validation
        behavior in putrequest (bpo-38216).
        """
        conn.sock = FakeSocket('')
        conn.putrequest('GET', '/\x00')

    def test_putrequest_override_encoding(self):
        """
        It should be possible to override the default encoding
        to transmit bytes in another encoding even if invalid