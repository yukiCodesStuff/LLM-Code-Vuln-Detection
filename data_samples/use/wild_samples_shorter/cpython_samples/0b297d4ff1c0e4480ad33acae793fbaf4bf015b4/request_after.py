
    # allow for double- and single-quoted realm values
    # (single quotes are a violation of the RFC, but appear in the wild)
    rx = re.compile('(?:^|,)'   # start of the string or ','
                    '[ \t]*'    # optional whitespaces
                    '([^ \t]+)' # scheme like "Basic"
                    '[ \t]+'    # mandatory whitespaces
                    # realm=xxx
                    # realm='xxx'
                    # realm="xxx"
                    'realm=(["\']?)([^"\']*)\\2',
                    re.I)

    # XXX could pre-emptively send auth info already accepted (RFC 2617,
    # end of section 2, and section 1.2 immediately after "credentials"
    # production).
        self.passwd = password_mgr
        self.add_password = self.passwd.add_password

    def _parse_realm(self, header):
        # parse WWW-Authenticate header: accept multiple challenges per header
        found_challenge = False
        for mo in AbstractBasicAuthHandler.rx.finditer(header):
            scheme, quote, realm = mo.groups()
            if quote not in ['"', "'"]:
                warnings.warn("Basic Auth Realm was unquoted",
                              UserWarning, 3)

            yield (scheme, realm)

            found_challenge = True

        if not found_challenge:
            if header:
                scheme = header.split()[0]
            else:
                scheme = ''
            yield (scheme, None)

    def http_error_auth_reqed(self, authreq, host, req, headers):
        # host may be an authority (without userinfo) or a URL with an
        # authority
        headers = headers.get_all(authreq)
        if not headers:
            # no header found
            return

        unsupported = None
        for header in headers:
            for scheme, realm in self._parse_realm(header):
                if scheme.lower() != 'basic':
                    unsupported = scheme
                    continue

                if realm is not None:
                    # Use the first matching Basic challenge.
                    # Ignore following challenges even if they use the Basic
                    # scheme.
                    return self.retry_http_basic_auth(host, req, realm)

        if unsupported is not None:
            raise ValueError("AbstractBasicAuthHandler does not "
                             "support the following scheme: %r"
                             % (scheme,))

    def retry_http_basic_auth(self, host, req, realm):
        user, pw = self.passwd.find_user_password(realm, host)
        if pw is not None: