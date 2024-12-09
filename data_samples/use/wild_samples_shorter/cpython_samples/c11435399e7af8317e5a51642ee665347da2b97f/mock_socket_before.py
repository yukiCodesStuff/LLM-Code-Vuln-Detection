    """
    def __init__(self, lines):
        self.lines = lines
    def readline(self):
        return self.lines.pop(0) + b'\r\n'
    def close(self):
        pass

