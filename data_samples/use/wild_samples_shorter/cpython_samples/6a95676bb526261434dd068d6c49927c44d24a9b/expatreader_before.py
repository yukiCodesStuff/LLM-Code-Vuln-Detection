            # FIXME: when to invoke error()?
            self._err_handler.fatalError(exc)

    def _close_source(self):
        source = self._source
        try:
            file = source.getCharacterStream()