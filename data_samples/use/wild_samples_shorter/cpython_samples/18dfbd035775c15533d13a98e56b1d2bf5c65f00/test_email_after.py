           [('Al Person', 'aperson@dom.ain'),
            ('Bud Person', 'bperson@dom.ain')])

    def test_getaddresses_parsing_errors(self):
        """Test for parsing errors from CVE-2023-27043"""
        eq = self.assertEqual
        eq(utils.getaddresses(['alice@example.org(<bob@example.com>']),
           [('', '')])
        eq(utils.getaddresses(['alice@example.org)<bob@example.com>']),
           [('', '')])
        eq(utils.getaddresses(['alice@example.org<<bob@example.com>']),
           [('', '')])
        eq(utils.getaddresses(['alice@example.org><bob@example.com>']),
           [('', '')])
        eq(utils.getaddresses(['alice@example.org@<bob@example.com>']),
           [('', '')])
        eq(utils.getaddresses(['alice@example.org,<bob@example.com>']),
           [('', 'alice@example.org'), ('', 'bob@example.com')])
        eq(utils.getaddresses(['alice@example.org;<bob@example.com>']),
           [('', '')])
        eq(utils.getaddresses(['alice@example.org:<bob@example.com>']),
           [('', '')])
        eq(utils.getaddresses(['alice@example.org.<bob@example.com>']),
           [('', '')])
        eq(utils.getaddresses(['alice@example.org"<bob@example.com>']),
           [('', '')])
        eq(utils.getaddresses(['alice@example.org[<bob@example.com>']),
           [('', '')])
        eq(utils.getaddresses(['alice@example.org]<bob@example.com>']),
           [('', '')])

    def test_parseaddr_parsing_errors(self):
        """Test for parsing errors from CVE-2023-27043"""
        eq = self.assertEqual
        eq(utils.parseaddr(['alice@example.org(<bob@example.com>']),
           ('', ''))
        eq(utils.parseaddr(['alice@example.org)<bob@example.com>']),
           ('', ''))
        eq(utils.parseaddr(['alice@example.org<<bob@example.com>']),
           ('', ''))
        eq(utils.parseaddr(['alice@example.org><bob@example.com>']),
           ('', ''))
        eq(utils.parseaddr(['alice@example.org@<bob@example.com>']),
           ('', ''))
        eq(utils.parseaddr(['alice@example.org,<bob@example.com>']),
           ('', ''))
        eq(utils.parseaddr(['alice@example.org;<bob@example.com>']),
           ('', ''))
        eq(utils.parseaddr(['alice@example.org:<bob@example.com>']),
           ('', ''))
        eq(utils.parseaddr(['alice@example.org.<bob@example.com>']),
           ('', ''))
        eq(utils.parseaddr(['alice@example.org"<bob@example.com>']),
           ('', ''))
        eq(utils.parseaddr(['alice@example.org[<bob@example.com>']),
           ('', ''))
        eq(utils.parseaddr(['alice@example.org]<bob@example.com>']),
           ('', ''))

    def test_getaddresses_nasty(self):
        eq = self.assertEqual
        eq(utils.getaddresses(['foo: ;']), [('', '')])
        eq(utils.getaddresses(['[]*-- =~$']), [('', '')])
        eq(utils.getaddresses(
           ['foo: ;', '"Jason R. Mastaler" <jason@dom.ain>']),
           [('', ''), ('Jason R. Mastaler', 'jason@dom.ain')])
        eq(utils.getaddresses(
           [r'Pete(A nice \) chap) <pete(his account)@silly.test(his host)>']),
           [('Pete (A nice ) chap his account his host)', 'pete@silly.test')])
        eq(utils.getaddresses(
           ['(Empty list)(start)Undisclosed recipients  :(nobody(I know))']),
           [('', '')])
        eq(utils.getaddresses(
           ['Mary <@machine.tld:mary@example.net>, , jdoe@test   . example']),
           [('Mary', 'mary@example.net'), ('', ''), ('', 'jdoe@test.example')])
        eq(utils.getaddresses(
           ['John Doe <jdoe@machine(comment).  example>']),
           [('John Doe (comment)', 'jdoe@machine.example')])
        eq(utils.getaddresses(
           ['"Mary Smith: Personal Account" <smith@home.example>']),
           [('Mary Smith: Personal Account', 'smith@home.example')])
        eq(utils.getaddresses(
           ['Undisclosed recipients:;']),
           [('', '')])
        eq(utils.getaddresses(
           [r'<boss@nil.test>, "Giant; \"Big\" Box" <bob@example.net>']),
           [('', 'boss@nil.test'), ('Giant; "Big" Box', 'bob@example.net')])

    def test_getaddresses_embedded_comment(self):
        """Test proper handling of a nested comment"""
        eq = self.assertEqual