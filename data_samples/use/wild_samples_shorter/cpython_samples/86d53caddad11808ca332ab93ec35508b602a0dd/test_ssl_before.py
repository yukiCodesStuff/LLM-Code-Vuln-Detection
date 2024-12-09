        self.assertRaises(ValueError, ssl.match_hostname, None, 'example.com')
        self.assertRaises(ValueError, ssl.match_hostname, {}, 'example.com')

    def test_server_side(self):
        # server_hostname doesn't work for server sockets
        ctx = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
        with socket.socket() as sock: