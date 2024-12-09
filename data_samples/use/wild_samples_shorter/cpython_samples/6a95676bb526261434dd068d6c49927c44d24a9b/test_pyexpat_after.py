        self.assertEqual(handler_call_args, [("bar", "baz")])


class ReparseDeferralTest(unittest.TestCase):
    def test_getter_setter_round_trip(self):
        parser = expat.ParserCreate()
        enabled = (expat.version_info >= (2, 6, 0))

        self.assertIs(parser.GetReparseDeferralEnabled(), enabled)
        parser.SetReparseDeferralEnabled(False)
        self.assertIs(parser.GetReparseDeferralEnabled(), False)
        parser.SetReparseDeferralEnabled(True)
        self.assertIs(parser.GetReparseDeferralEnabled(), enabled)

    def test_reparse_deferral_enabled(self):
        if expat.version_info < (2, 6, 0):
            self.skipTest(f'Expat {expat.version_info} does not '
                          'support reparse deferral')

        started = []

        def start_element(name, _):
            started.append(name)

        parser = expat.ParserCreate()
        parser.StartElementHandler = start_element
        self.assertTrue(parser.GetReparseDeferralEnabled())

        for chunk in (b'<doc', b'/>'):
            parser.Parse(chunk, False)

        # The key test: Have handlers already fired?  Expecting: no.
        self.assertEqual(started, [])

        parser.Parse(b'', True)

        self.assertEqual(started, ['doc'])

    def test_reparse_deferral_disabled(self):
        started = []

        def start_element(name, _):
            started.append(name)

        parser = expat.ParserCreate()
        parser.StartElementHandler = start_element
        if expat.version_info >= (2, 6, 0):
            parser.SetReparseDeferralEnabled(False)
        self.assertFalse(parser.GetReparseDeferralEnabled())

        for chunk in (b'<doc', b'/>'):
            parser.Parse(chunk, False)

        # The key test: Have handlers already fired?  Expecting: yes.
        self.assertEqual(started, ['doc'])


if __name__ == "__main__":
    unittest.main()