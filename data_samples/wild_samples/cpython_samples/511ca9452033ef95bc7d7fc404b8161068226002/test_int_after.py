    def test_issue31619(self):
        self.assertEqual(int('1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1', 2),
                         0b1010101010101010101010101010101)
        self.assertEqual(int('1_2_3_4_5_6_7_0_1_2_3', 8), 0o12345670123)
        self.assertEqual(int('1_2_3_4_5_6_7_8_9', 16), 0x123456789)
        self.assertEqual(int('1_2_3_4_5_6_7', 32), 1144132807)


class IntStrDigitLimitsTests(unittest.TestCase):

    int_class = int  # Override this in subclasses to reuse the suite.

    def setUp(self):
        super().setUp()
        self._previous_limit = sys.get_int_max_str_digits()
        sys.set_int_max_str_digits(2048)

    def tearDown(self):
        sys.set_int_max_str_digits(self._previous_limit)
        super().tearDown()

    def test_disabled_limit(self):
        self.assertGreater(sys.get_int_max_str_digits(), 0)
        self.assertLess(sys.get_int_max_str_digits(), 20_000)
        with support.adjust_int_max_str_digits(0):
            self.assertEqual(sys.get_int_max_str_digits(), 0)
            i = self.int_class('1' * 20_000)
            str(i)
        self.assertGreater(sys.get_int_max_str_digits(), 0)

    def test_max_str_digits_edge_cases(self):
        """Ignore the +/- sign and space padding."""
        int_class = self.int_class
        maxdigits = sys.get_int_max_str_digits()

        int_class('1' * maxdigits)
        int_class(' ' + '1' * maxdigits)
        int_class('1' * maxdigits + ' ')
        int_class('+' + '1' * maxdigits)
        int_class('-' + '1' * maxdigits)
        self.assertEqual(len(str(10 ** (maxdigits - 1))), maxdigits)

    def check(self, i, base=None):
        with self.assertRaises(ValueError):
            if base is None:
                self.int_class(i)
            else:
                self.int_class(i, base)

    def test_max_str_digits(self):
        maxdigits = sys.get_int_max_str_digits()

        self.check('1' * (maxdigits + 1))
        self.check(' ' + '1' * (maxdigits + 1))
        self.check('1' * (maxdigits + 1) + ' ')
        self.check('+' + '1' * (maxdigits + 1))
        self.check('-' + '1' * (maxdigits + 1))
        self.check('1' * (maxdigits + 1))

        i = 10 ** maxdigits
        with self.assertRaises(ValueError):
            str(i)

    def test_power_of_two_bases_unlimited(self):
        """The limit does not apply to power of 2 bases."""
        maxdigits = sys.get_int_max_str_digits()

        for base in (2, 4, 8, 16, 32):
            with self.subTest(base=base):
                self.int_class('1' * (maxdigits + 1), base)
                assert maxdigits < 100_000
                self.int_class('1' * 100_000, base)

    def test_underscores_ignored(self):
        maxdigits = sys.get_int_max_str_digits()

        triples = maxdigits // 3
        s = '111' * triples
        s_ = '1_11' * triples
        self.int_class(s)  # succeeds
        self.int_class(s_)  # succeeds
        self.check(f'{s}111')
        self.check(f'{s_}_111')

    def test_sign_not_counted(self):
        int_class = self.int_class
        max_digits = sys.get_int_max_str_digits()
        s = '5' * max_digits
        i = int_class(s)
        pos_i = int_class(f'+{s}')
        assert i == pos_i
        neg_i = int_class(f'-{s}')
        assert -pos_i == neg_i
        str(pos_i)
        str(neg_i)

    def _other_base_helper(self, base):
        int_class = self.int_class
        max_digits = sys.get_int_max_str_digits()
        s = '2' * max_digits
        i = int_class(s, base)
        if base > 10:
            with self.assertRaises(ValueError):
                str(i)
        elif base < 10:
            str(i)
        with self.assertRaises(ValueError) as err:
            int_class(f'{s}1', base)

    def test_int_from_other_bases(self):
        base = 3
        with self.subTest(base=base):
            self._other_base_helper(base)
        base = 36
        with self.subTest(base=base):
            self._other_base_helper(base)


class IntSubclassStrDigitLimitsTests(IntStrDigitLimitsTests):
    int_class = IntSubclass


if __name__ == "__main__":
    unittest.main()