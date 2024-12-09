        )
        self = cls.__new__(cls, **kwargs)
        super(SSLSocket, self).__init__(**kwargs)
        sock_timeout = sock.gettimeout()
        sock.detach()

        self._context = context
        self._session = session
            if e.errno != errno.ENOTCONN:
                raise
            connected = False
            blocking = self.getblocking()
            self.setblocking(False)
            try:
                # We are not connected so this is not supposed to block, but
                # testing revealed otherwise on macOS and Windows so we do
                # the non-blocking dance regardless. Our raise when any data
                # is found means consuming the data is harmless.
                notconn_pre_handshake_data = self.recv(1)
            except OSError as e:
                # EINVAL occurs for recv(1) on non-connected on unix sockets.
                if e.errno not in (errno.ENOTCONN, errno.EINVAL):
                    raise
                notconn_pre_handshake_data = b''
            self.setblocking(blocking)
            if notconn_pre_handshake_data:
                # This prevents pending data sent to the socket before it was
                # closed from escaping to the caller who could otherwise
                # presume it came through a successful TLS connection.
                reason = "Closed before TLS handshake with data in recv buffer."
                notconn_pre_handshake_data_error = SSLError(e.errno, reason)
                # Add the SSLError attributes that _ssl.c always adds.
                notconn_pre_handshake_data_error.reason = reason
                notconn_pre_handshake_data_error.library = None
                try:
                    self.close()
                except OSError:
                    pass
                raise notconn_pre_handshake_data_error
        else:
            connected = True

        self.settimeout(sock_timeout)  # Must come after setblocking() calls.
        self._connected = connected
        if connected:
            # create the SSL object
            try: