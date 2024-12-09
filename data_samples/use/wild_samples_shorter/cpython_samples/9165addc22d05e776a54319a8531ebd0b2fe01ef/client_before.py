
        (self.host, self.port) = self._get_hostport(host, port)

        # This is stored as an instance variable to allow unit
        # tests to replace it with a suitable mockup
        self._create_connection = socket.create_connection

            raise InvalidURL(f"URL can't contain control characters. {url!r} "
                             f"(found at least {match.group()!r})")

    def putheader(self, header, *values):
        """Send a request header line to the server.

        For example: h.putheader('Accept', 'text/html')