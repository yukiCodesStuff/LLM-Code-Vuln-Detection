from io import StringIO
from collections import OrderedDict
from test.test_json import PyTest, CTest
from test import support


class TestDecode:
    def test_decimal(self):
        d = self.json.JSONDecoder()
        self.assertRaises(ValueError, d.raw_decode, 'a'*42, -50000)

    def test_limit_int(self):
        maxdigits = 5000
        with support.adjust_int_max_str_digits(maxdigits):
            self.loads('1' * maxdigits)
            with self.assertRaises(ValueError):
                self.loads('1' * (maxdigits + 1))


class TestPyDecode(TestDecode, PyTest): pass
class TestCDecode(TestDecode, CTest): pass