from io import StringIO
from collections import OrderedDict
from test.test_json import PyTest, CTest


class TestDecode:
    def test_decimal(self):
        d = self.json.JSONDecoder()
        self.assertRaises(ValueError, d.raw_decode, 'a'*42, -50000)

class TestPyDecode(TestDecode, PyTest): pass
class TestCDecode(TestDecode, CTest): pass