           "decode_header",
           ]

# maximal line length when calling readline(). This is to prevent
# reading arbitrary lenght lines. RFC 3977 limits NNTP line length to
# 512 characters, including CRLF. We have selected 2048 just to be on
# the safe side.
_MAXLINE = 2048


# Exceptions raised when an error or invalid response is received
class NNTPError(Exception):
    """Base class for all nntplib exceptions"""
    def __init__(self, *args):
        """Internal: return one line from the server, stripping _CRLF.
        Raise EOFError if the connection is closed.
        Returns a bytes object."""
        line = self.file.readline(_MAXLINE +1)
        if len(line) > _MAXLINE:
            raise NNTPDataError('line too long')
        if self.debugging > 1:
            print('*get*', repr(line))
        if not line: raise EOFError
        if strip_crlf: