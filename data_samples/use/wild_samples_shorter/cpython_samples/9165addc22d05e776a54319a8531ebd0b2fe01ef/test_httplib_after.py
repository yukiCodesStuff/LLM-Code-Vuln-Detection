        thread.join()
        self.assertEqual(result, b"proxied data\n")

    def test_putrequest_override_domain_validation(self):
        """
        It should be possible to override the default validation
        behavior in putrequest (bpo-38216).
        """
        conn.sock = FakeSocket('')
        conn.putrequest('GET', '/\x00')

    def test_putrequest_override_host_validation(self):
        class UnsafeHTTPConnection(client.HTTPConnection):
            def _validate_host(self, url):
                pass

        conn = UnsafeHTTPConnection('example.com\r\n')
        conn.sock = FakeSocket('')
        # set skip_host so a ValueError is not raised upon adding the
        # invalid URL as the value of the "Host:" header
        conn.putrequest('GET', '/', skip_host=1)

    def test_putrequest_override_encoding(self):
        """
        It should be possible to override the default encoding
        to transmit bytes in another encoding even if invalid