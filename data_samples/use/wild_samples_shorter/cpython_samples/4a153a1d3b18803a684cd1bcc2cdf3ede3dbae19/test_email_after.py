
import email
import email.policy
import email.utils

from email.charset import Charset
from email.generator import Generator, DecodedGenerator, BytesGenerator
from email.header import Header, decode_header, make_header
            ],
        )

    def test_parsing_errors(self):
        """Test for parsing errors from CVE-2023-27043 and CVE-2019-16056"""
        alice = 'alice@example.org'
        bob = 'bob@example.com'
        empty = ('', '')

        # Test utils.getaddresses() and utils.parseaddr() on malformed email
        # addresses: default behavior (strict=True) rejects malformed address,
        # and strict=False which tolerates malformed address.
        for invalid_separator, expected_non_strict in (
            ('(', [(f'<{bob}>', alice)]),
            (')', [('', alice), empty, ('', bob)]),
            ('<', [('', alice), empty, ('', bob), empty]),
            ('>', [('', alice), empty, ('', bob)]),
            ('[', [('', f'{alice}[<{bob}>]')]),
            (']', [('', alice), empty, ('', bob)]),
            ('@', [empty, empty, ('', bob)]),
            (';', [('', alice), empty, ('', bob)]),
            (':', [('', alice), ('', bob)]),
            ('.', [('', alice + '.'), ('', bob)]),
            ('"', [('', alice), ('', f'<{bob}>')]),
        ):
            address = f'{alice}{invalid_separator}<{bob}>'
            with self.subTest(address=address):
                self.assertEqual(utils.getaddresses([address]),
                                 [empty])
                self.assertEqual(utils.getaddresses([address], strict=False),
                                 expected_non_strict)

                self.assertEqual(utils.parseaddr([address]),
                                 empty)
                self.assertEqual(utils.parseaddr([address], strict=False),
                                 ('', address))

        # Comma (',') is treated differently depending on strict parameter.
        # Comma without quotes.
        address = f'{alice},<{bob}>'
        self.assertEqual(utils.getaddresses([address]),
                         [('', alice), ('', bob)])
        self.assertEqual(utils.getaddresses([address], strict=False),
                         [('', alice), ('', bob)])
        self.assertEqual(utils.parseaddr([address]),
                         empty)
        self.assertEqual(utils.parseaddr([address], strict=False),
                         ('', address))

        # Real name between quotes containing comma.
        address = '"Alice, alice@example.org" <bob@example.com>'
        expected_strict = ('Alice, alice@example.org', 'bob@example.com')
        self.assertEqual(utils.getaddresses([address]), [expected_strict])
        self.assertEqual(utils.getaddresses([address], strict=False), [expected_strict])
        self.assertEqual(utils.parseaddr([address]), expected_strict)
        self.assertEqual(utils.parseaddr([address], strict=False),
                         ('', address))

        # Valid parenthesis in comments.
        address = 'alice@example.org (Alice)'
        expected_strict = ('Alice', 'alice@example.org')
        self.assertEqual(utils.getaddresses([address]), [expected_strict])
        self.assertEqual(utils.getaddresses([address], strict=False), [expected_strict])
        self.assertEqual(utils.parseaddr([address]), expected_strict)
        self.assertEqual(utils.parseaddr([address], strict=False),
                         ('', address))

        # Invalid parenthesis in comments.
        address = 'alice@example.org )Alice('
        self.assertEqual(utils.getaddresses([address]), [empty])
        self.assertEqual(utils.getaddresses([address], strict=False),
                         [('', 'alice@example.org'), ('', ''), ('', 'Alice')])
        self.assertEqual(utils.parseaddr([address]), empty)
        self.assertEqual(utils.parseaddr([address], strict=False),
                         ('', address))

        # Two addresses with quotes separated by comma.
        address = '"Jane Doe" <jane@example.net>, "John Doe" <john@example.net>'
        self.assertEqual(utils.getaddresses([address]),
                         [('Jane Doe', 'jane@example.net'),
                          ('John Doe', 'john@example.net')])
        self.assertEqual(utils.getaddresses([address], strict=False),
                         [('Jane Doe', 'jane@example.net'),
                          ('John Doe', 'john@example.net')])
        self.assertEqual(utils.parseaddr([address]), empty)
        self.assertEqual(utils.parseaddr([address], strict=False),
                         ('', address))

        # Test email.utils.supports_strict_parsing attribute
        self.assertEqual(email.utils.supports_strict_parsing, True)

    def test_getaddresses_nasty(self):
        for addresses, expected in (
            (['"Sürname, Firstname" <to@example.com>'],
             [('Sürname, Firstname', 'to@example.com')]),

            (['foo: ;'],
             [('', '')]),

            (['foo: ;', '"Jason R. Mastaler" <jason@dom.ain>'],
             [('', ''), ('Jason R. Mastaler', 'jason@dom.ain')]),

            ([r'Pete(A nice \) chap) <pete(his account)@silly.test(his host)>'],
             [('Pete (A nice ) chap his account his host)', 'pete@silly.test')]),

            (['(Empty list)(start)Undisclosed recipients  :(nobody(I know))'],
             [('', '')]),

            (['Mary <@machine.tld:mary@example.net>, , jdoe@test   . example'],
             [('Mary', 'mary@example.net'), ('', ''), ('', 'jdoe@test.example')]),

            (['John Doe <jdoe@machine(comment).  example>'],
             [('John Doe (comment)', 'jdoe@machine.example')]),

            (['"Mary Smith: Personal Account" <smith@home.example>'],
             [('Mary Smith: Personal Account', 'smith@home.example')]),

            (['Undisclosed recipients:;'],
             [('', '')]),

            ([r'<boss@nil.test>, "Giant; \"Big\" Box" <bob@example.net>'],
             [('', 'boss@nil.test'), ('Giant; "Big" Box', 'bob@example.net')]),
        ):
            with self.subTest(addresses=addresses):
                self.assertEqual(utils.getaddresses(addresses),
                                 expected)
                self.assertEqual(utils.getaddresses(addresses, strict=False),
                                 expected)

        addresses = ['[]*-- =~$']
        self.assertEqual(utils.getaddresses(addresses),
                         [('', '')])
        self.assertEqual(utils.getaddresses(addresses, strict=False),
                         [('', ''), ('', ''), ('', '*--')])

    def test_getaddresses_embedded_comment(self):
        """Test proper handling of a nested comment"""
        eq = self.assertEqual
                m = cls(*constructor, policy=email.policy.default)
                self.assertIs(m.policy, email.policy.default)

    def test_iter_escaped_chars(self):
        self.assertEqual(list(utils._iter_escaped_chars(r'a\\b\"c\\"d')),
                         [(0, 'a'),
                          (2, '\\\\'),
                          (3, 'b'),
                          (5, '\\"'),
                          (6, 'c'),
                          (8, '\\\\'),
                          (9, '"'),
                          (10, 'd')])
        self.assertEqual(list(utils._iter_escaped_chars('a\\')),
                         [(0, 'a'), (1, '\\')])

    def test_strip_quoted_realnames(self):
        def check(addr, expected):
            self.assertEqual(utils._strip_quoted_realnames(addr), expected)

        check('"Jane Doe" <jane@example.net>, "John Doe" <john@example.net>',
              ' <jane@example.net>,  <john@example.net>')
        check(r'"Jane \"Doe\"." <jane@example.net>',
              ' <jane@example.net>')

        # special cases
        check(r'before"name"after', 'beforeafter')
        check(r'before"name"', 'before')
        check(r'b"name"', 'b')  # single char
        check(r'"name"after', 'after')
        check(r'"name"a', 'a')  # single char
        check(r'"name"', '')

        # no change
        for addr in (
            'Jane Doe <jane@example.net>, John Doe <john@example.net>',
            'lone " quote',
        ):
            self.assertEqual(utils._strip_quoted_realnames(addr), addr)


    def test_check_parenthesis(self):
        addr = 'alice@example.net'
        self.assertTrue(utils._check_parenthesis(f'{addr} (Alice)'))
        self.assertFalse(utils._check_parenthesis(f'{addr} )Alice('))
        self.assertFalse(utils._check_parenthesis(f'{addr} (Alice))'))
        self.assertFalse(utils._check_parenthesis(f'{addr} ((Alice)'))

        # Ignore real name between quotes
        self.assertTrue(utils._check_parenthesis(f'")Alice((" {addr}'))


# Test the iterator/generators
class TestIterators(TestEmailBase):
    def test_body_line_iterator(self):