            else:
                yield event

    def flush(self):
        if self._parser is None:
            raise ValueError("flush() called after end of stream")
        self._parser.flush()


def XML(text, parser=None):
    """Parse XML document from string constant.

            del self.parser, self._parser
            del self.target, self._target

    def flush(self):
        was_enabled = self.parser.GetReparseDeferralEnabled()
        try:
            self.parser.SetReparseDeferralEnabled(False)
            self.parser.Parse(b"", False)
        except self._error as v:
            self._raiseerror(v)
        finally:
            self.parser.SetReparseDeferralEnabled(was_enabled)

# --------------------------------------------------------------------
# C14N 2.0
