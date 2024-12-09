            list(generator(*args))
        self.assertEqual(msg, str(ctx.exception))

class TestJunkAPIs(unittest.TestCase):
    def test_is_line_junk_true(self):
        for line in ['#', '  ', ' #', '# ', ' # ', '']:
            self.assertTrue(difflib.IS_LINE_JUNK(line), repr(line))

    def test_is_line_junk_false(self):
        for line in ['##', ' ##', '## ', 'abc ', 'abc #', 'Mr. Moose is up!']:
            self.assertFalse(difflib.IS_LINE_JUNK(line), repr(line))

    def test_is_line_junk_REDOS(self):
        evil_input = ('\t' * 1000000) + '##'
        self.assertFalse(difflib.IS_LINE_JUNK(evil_input))

    def test_is_character_junk_true(self):
        for char in [' ', '\t']:
            self.assertTrue(difflib.IS_CHARACTER_JUNK(char), repr(char))

    def test_is_character_junk_false(self):
        for char in ['a', '#', '\n', '\f', '\r', '\v']:
            self.assertFalse(difflib.IS_CHARACTER_JUNK(char), repr(char))

def test_main():
    difflib.HtmlDiff._default_prefix = 0
    Doctests = doctest.DocTestSuite(difflib)
    run_unittest(
        TestWithAscii, TestAutojunk, TestSFpatches, TestSFbugs,
        TestOutputFormat, TestBytes, TestJunkAPIs, Doctests)

if __name__ == '__main__':
    test_main()