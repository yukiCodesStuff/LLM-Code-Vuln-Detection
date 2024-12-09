IMAP4_SSL_PORT = 993
AllowedVersions = ('IMAP4REV1', 'IMAP4')        # Most recent first

# Maximal line length when calling readline(). This is to prevent
# reading arbitrary length lines. RFC 3501 and 2060 (IMAP 4rev1)
# don't specify a line length. RFC 2683 however suggests limiting client
# command lines to 1000 octets and server command lines to 8000 octets.
# We have selected 10000 for some extra margin and since that is supposedly
# also what UW and Panda IMAP does.
_MAXLINE = 10000


#       Commands

Commands = {
        # name            valid states

    def readline(self):
        """Read line from remote."""
        line = self.file.readline(_MAXLINE + 1)
        if len(line) > _MAXLINE:
            raise self.error("got more than %d bytes" % _MAXLINE)
        return line


    def send(self, data):
        """Send data to remote."""