IMAP4_SSL_PORT = 993
AllowedVersions = ('IMAP4REV1', 'IMAP4')        # Most recent first

#       Commands

Commands = {
        # name            valid states

    def readline(self):
        """Read line from remote."""
        return self.file.readline()


    def send(self, data):
        """Send data to remote."""