        check('<bigdecimal>9876543210.0123456789</bigdecimal>',
              decimal.Decimal('9876543210.0123456789'))

    def test_limit_int(self):
        check = self.check_loads
        maxdigits = 5000
        with support.adjust_int_max_str_digits(maxdigits):
            s = '1' * (maxdigits + 1)
            with self.assertRaises(ValueError):
                check(f'<int>{s}</int>', None)
            with self.assertRaises(ValueError):
                check(f'<biginteger>{s}</biginteger>', None)

    def test_get_host_info(self):
        # see bug #3613, this raised a TypeError
        transp = xmlrpc.client.Transport()
        self.assertEqual(transp.get_host_info("user@host.tld"),