        check('<bigdecimal>9876543210.0123456789</bigdecimal>',
              decimal.Decimal('9876543210.0123456789'))

    def test_get_host_info(self):
        # see bug #3613, this raised a TypeError
        transp = xmlrpc.client.Transport()
        self.assertEqual(transp.get_host_info("user@host.tld"),