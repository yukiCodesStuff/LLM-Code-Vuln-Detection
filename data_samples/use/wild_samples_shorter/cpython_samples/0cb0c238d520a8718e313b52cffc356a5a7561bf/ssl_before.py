        )
        self = cls.__new__(cls, **kwargs)
        super(SSLSocket, self).__init__(**kwargs)
        self.settimeout(sock.gettimeout())
        sock.detach()

        self._context = context
        self._session = session
            if e.errno != errno.ENOTCONN:
                raise
            connected = False
        else:
            connected = True

        self._connected = connected
        if connected:
            # create the SSL object
            try: