
# The standard FTP server control port
FTP_PORT = 21


# Exception raised when an error or invalid response is received
class Error(Exception): pass
    debugging = 0
    host = ''
    port = FTP_PORT
    sock = None
    file = None
    welcome = None
    passiveserver = 1
    # Internal: return one line from the server, stripping CRLF.
    # Raise EOFError if the connection is closed
    def getline(self):
        line = self.file.readline()
        if self.debugging > 1:
            print('*get*', self.sanitize(line))
        if not line:
            raise EOFError
        with self.transfercmd(cmd) as conn, \
                 conn.makefile('r', encoding=self.encoding) as fp:
            while 1:
                line = fp.readline()
                if self.debugging > 2:
                    print('*retr*', repr(line))
                if not line:
                    break
        self.voidcmd('TYPE A')
        with self.transfercmd(cmd) as conn:
            while 1:
                buf = fp.readline()
                if not buf:
                    break
                if buf[-2:] != B_CRLF:
                    if buf[-1] in B_CRLF: buf = buf[:-1]