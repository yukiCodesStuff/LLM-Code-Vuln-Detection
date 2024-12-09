SMTP_SSL_PORT = 465
CRLF = "\r\n"
bCRLF = b"\r\n"

OLDSTYLE_AUTH = re.compile(r"auth=(.*)", re.I)

# Exception classes used by this module.
            self.file = self.sock.makefile('rb')
        while 1:
            try:
                line = self.file.readline()
            except socket.error as e:
                self.close()
                raise SMTPServerDisconnected("Connection unexpectedly closed: "
                                             + str(e))
                raise SMTPServerDisconnected("Connection unexpectedly closed")
            if self.debuglevel > 0:
                print('reply:', repr(line), file=stderr)
            resp.append(line[4:].strip(b' \t\r\n'))
            code = line[:3]
            # Check that the error code is syntactically correct.
            # Don't attempt to read a continuation line if it is broken.