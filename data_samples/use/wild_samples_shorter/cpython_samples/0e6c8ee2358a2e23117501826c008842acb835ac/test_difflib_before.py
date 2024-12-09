            list(generator(*args))
        self.assertEqual(msg, str(ctx.exception))


def test_main():
    difflib.HtmlDiff._default_prefix = 0
    Doctests = doctest.DocTestSuite(difflib)
    run_unittest(
        TestWithAscii, TestAutojunk, TestSFpatches, TestSFbugs,
        TestOutputFormat, TestBytes, Doctests)

if __name__ == '__main__':
    test_main()