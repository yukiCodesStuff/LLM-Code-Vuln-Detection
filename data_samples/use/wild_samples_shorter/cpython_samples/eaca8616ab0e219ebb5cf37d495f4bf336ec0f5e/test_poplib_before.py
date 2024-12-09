
    def cmd_list(self, arg):
        if arg:
            self.push('+OK %s %s' %(arg, arg))
        else:
            self.push('+OK')
            asynchat.async_chat.push(self, LIST_RESP)

        foo = self.client.retr('foo')
        self.assertEqual(foo, expected)

    def test_dele(self):
        self.assertOK(self.client.dele('foo'))

    def test_noop(self):