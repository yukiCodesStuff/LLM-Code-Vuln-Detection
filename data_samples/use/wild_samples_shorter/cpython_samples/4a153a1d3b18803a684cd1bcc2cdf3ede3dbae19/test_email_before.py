
import email
import email.policy

from email.charset import Charset
from email.generator import Generator, DecodedGenerator, BytesGenerator
from email.header import Header, decode_header, make_header
            ],
        )

    def test_getaddresses_nasty(self):
        eq = self.assertEqual
        eq(utils.getaddresses(['foo: ;']), [('', '')])
        eq(utils.getaddresses(
           ['[]*-- =~$']),
           [('', ''), ('', ''), ('', '*--')])
        eq(utils.getaddresses(
           ['foo: ;', '"Jason R. Mastaler" <jason@dom.ain>']),
           [('', ''), ('Jason R. Mastaler', 'jason@dom.ain')])

    def test_getaddresses_embedded_comment(self):
        """Test proper handling of a nested comment"""
        eq = self.assertEqual
                m = cls(*constructor, policy=email.policy.default)
                self.assertIs(m.policy, email.policy.default)


# Test the iterator/generators
class TestIterators(TestEmailBase):
    def test_body_line_iterator(self):