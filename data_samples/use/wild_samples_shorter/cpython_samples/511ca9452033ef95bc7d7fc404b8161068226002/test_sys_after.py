        self.assertIsInstance(sys.executable, str)
        self.assertEqual(len(sys.float_info), 11)
        self.assertEqual(sys.float_info.radix, 2)
        self.assertEqual(len(sys.int_info), 4)
        self.assertTrue(sys.int_info.bits_per_digit % 5 == 0)
        self.assertTrue(sys.int_info.sizeof_digit >= 1)
        self.assertGreaterEqual(sys.int_info.default_max_str_digits, 500)
        self.assertGreaterEqual(sys.int_info.str_digits_check_threshold, 100)
        self.assertGreater(sys.int_info.default_max_str_digits,
                           sys.int_info.str_digits_check_threshold)
        self.assertEqual(type(sys.int_info.bits_per_digit), int)
        self.assertEqual(type(sys.int_info.sizeof_digit), int)
        self.assertIsInstance(sys.int_info.default_max_str_digits, int)
        self.assertIsInstance(sys.int_info.str_digits_check_threshold, int)
        self.assertIsInstance(sys.hexversion, int)

        self.assertEqual(len(sys.hash_info), 9)
        self.assertLess(sys.hash_info.modulus, 2**sys.hash_info.width)
                 "dont_write_bytecode", "no_user_site", "no_site",
                 "ignore_environment", "verbose", "bytes_warning", "quiet",
                 "hash_randomization", "isolated", "dev_mode", "utf8_mode",
                 "warn_default_encoding", "safe_path", "int_max_str_digits")
        for attr in attrs:
            self.assertTrue(hasattr(sys.flags, attr), attr)
            attr_type = bool if attr in ("dev_mode", "safe_path") else int
            self.assertEqual(type(getattr(sys.flags, attr)), attr_type, attr)