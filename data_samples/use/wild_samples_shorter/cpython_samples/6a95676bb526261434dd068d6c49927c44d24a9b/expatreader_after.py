            # FIXME: when to invoke error()?
            self._err_handler.fatalError(exc)

    def flush(self):
        if self._parser is None:
            return

        was_enabled = self._parser.GetReparseDeferralEnabled()
        try:
            self._parser.SetReparseDeferralEnabled(False)
            self._parser.Parse(b"", False)
        except expat.error as e:
            exc = SAXParseException(expat.ErrorString(e.code), e, self)
            self._err_handler.fatalError(exc)
        finally:
            self._parser.SetReparseDeferralEnabled(was_enabled)

    def _close_source(self):
        source = self._source
        try:
            file = source.getCharacterStream()