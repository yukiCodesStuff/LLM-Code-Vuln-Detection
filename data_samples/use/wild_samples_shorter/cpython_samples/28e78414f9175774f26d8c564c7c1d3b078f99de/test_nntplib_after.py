                <a4929a40-6328-491a-aaaf-cb79ed7309a2@q2g2000vbk.googlegroups.com>
                <f30c0419-f549-4218-848f-d7d0131da931@y3g2000vbm.googlegroups.com>
                .""")
        elif (group == 'comp.lang.python' and
              date_str in ('20100101', '100101') and
              time_str == '090000'):
            self.push_lit('too long line' * 3000 +
                          '\n.')
        else:
            self.push_lit("""\
                230 An empty list of newsarticles follows
                .""")
        self.assertEqual(cm.exception.response,
                         "435 Article not wanted")

    def test_too_long_lines(self):
        dt = datetime.datetime(2010, 1, 1, 9, 0, 0)
        self.assertRaises(nntplib.NNTPDataError,
                          self.server.newnews, "comp.lang.python", dt)


class NNTPv1Tests(NNTPv1v2TestsMixin, MockedNNTPTestsMixin, unittest.TestCase):
    """Tests an NNTP v1 server (no capabilities)."""
