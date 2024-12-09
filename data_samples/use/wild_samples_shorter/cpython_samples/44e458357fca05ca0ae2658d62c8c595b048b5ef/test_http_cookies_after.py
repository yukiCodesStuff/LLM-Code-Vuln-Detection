import doctest
from http import cookies
import pickle
from test import support


class CookieTests(unittest.TestCase):

            for k, v in sorted(case['dict'].items()):
                self.assertEqual(C[k].value, v)

    def test_unquote(self):
        cases = [
            (r'a="b=\""', 'b="'),
            (r'a="b=\\"', 'b=\\'),
            (r'a="b=\="', 'b=='),
            (r'a="b=\n"', 'b=n'),
            (r'a="b=\042"', 'b="'),
            (r'a="b=\134"', 'b=\\'),
            (r'a="b=\377"', 'b=\xff'),
            (r'a="b=\400"', 'b=400'),
            (r'a="b=\42"', 'b=42'),
            (r'a="b=\\042"', 'b=\\042'),
            (r'a="b=\\134"', 'b=\\134'),
            (r'a="b=\\\""', 'b=\\"'),
            (r'a="b=\\\042"', 'b=\\"'),
            (r'a="b=\134\""', 'b=\\"'),
            (r'a="b=\134\042"', 'b=\\"'),
        ]
        for encoded, decoded in cases:
            with self.subTest(encoded):
                C = cookies.SimpleCookie()
                C.load(encoded)
                self.assertEqual(C['a'].value, decoded)

    @support.requires_resource('cpu')
    def test_unquote_large(self):
        n = 10**6
        for encoded in r'\\', r'\134':
            with self.subTest(encoded):
                data = 'a="b=' + encoded*n + ';"'
                C = cookies.SimpleCookie()
                C.load(data)
                value = C['a'].value
                self.assertEqual(value[:3], 'b=\\')
                self.assertEqual(value[-2:], '\\;')
                self.assertEqual(len(value), n + 3)

    def test_load(self):
        C = cookies.SimpleCookie()
        C.load('Customer="WILE_E_COYOTE"; Version=1; Path=/acme')
