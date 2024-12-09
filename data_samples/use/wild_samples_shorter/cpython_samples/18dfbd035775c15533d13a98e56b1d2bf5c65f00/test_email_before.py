           [('Al Person', 'aperson@dom.ain'),
            ('Bud Person', 'bperson@dom.ain')])

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