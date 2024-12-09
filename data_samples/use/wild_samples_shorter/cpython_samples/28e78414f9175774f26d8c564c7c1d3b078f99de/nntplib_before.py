           "decode_header",
           ]

# Exceptions raised when an error or invalid response is received
class NNTPError(Exception):
    """Base class for all nntplib exceptions"""
    def __init__(self, *args):
        """Internal: return one line from the server, stripping _CRLF.
        Raise EOFError if the connection is closed.
        Returns a bytes object."""
        line = self.file.readline()
        if self.debugging > 1:
            print('*get*', repr(line))
        if not line: raise EOFError
        if strip_crlf: