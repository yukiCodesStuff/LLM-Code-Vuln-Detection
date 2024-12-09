    """
    def __init__(self, lines):
        self.lines = lines
    def readline(self, limit=-1):
        result = self.lines.pop(0) + b'\r\n'
        if limit >= 0:
            # Re-insert the line, removing the \r\n we added.
            self.lines.insert(0, result[limit:-2])
            result = result[:limit]
        return result
    def close(self):
        pass

