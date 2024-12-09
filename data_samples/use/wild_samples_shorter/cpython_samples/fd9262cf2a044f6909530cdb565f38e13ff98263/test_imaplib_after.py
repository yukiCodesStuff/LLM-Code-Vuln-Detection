            self.assertEqual(ret, "OK")


    def test_linetoolong(self):
        class TooLongHandler(SimpleIMAPHandler):
            def handle(self):
                # Send a very long response line
                self.wfile.write(b'* OK ' + imaplib._MAXLINE*b'x' + b'\r\n')

        with self.reaped_server(TooLongHandler) as server:
            self.assertRaises(imaplib.IMAP4.error,
                              self.imap_class, *server.server_address)


class ThreadedNetworkedTests(BaseThreadedNetworkedTests):

    server_class = socketserver.TCPServer
    imap_class = imaplib.IMAP4