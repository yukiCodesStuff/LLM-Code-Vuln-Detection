LF = b'\n'
CRLF = CR+LF


class POP3:

    """This class supports both the minimal and optional command sets.
    # Raise error_proto('-ERR EOF') if the connection is closed.

    def _getline(self):
        line = self.file.readline()
        if self._debugging > 1: print('*get*', repr(line))
        if not line: raise error_proto('-ERR EOF')
        octets = len(line)
        # server can send any combination of CR & LF