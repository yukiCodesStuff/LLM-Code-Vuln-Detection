
    # allow for double- and single-quoted realm values
    # (single quotes are a violation of the RFC, but appear in the wild)
    rx = re.compile('(?:.*,)*[ \t]*([^ \t]+)[ \t]+'
                    'realm=(["\']?)([^"\']*)\\2', re.I)

    # XXX could pre-emptively send auth info already accepted (RFC 2617,
    # end of section 2, and section 1.2 immediately after "credentials"
    # production).
        self.passwd = password_mgr
        self.add_password = self.passwd.add_password

    def http_error_auth_reqed(self, authreq, host, req, headers):
        # host may be an authority (without userinfo) or a URL with an
        # authority
        # XXX could be multiple headers
        authreq = headers.get(authreq, None)

        if authreq:
            scheme = authreq.split()[0]
            if scheme.lower() != 'basic':
                raise ValueError("AbstractBasicAuthHandler does not"
                                 " support the following scheme: '%s'" %
                                 scheme)
            else:
                mo = AbstractBasicAuthHandler.rx.search(authreq)
                if mo:
                    scheme, quote, realm = mo.groups()
                    if quote not in ['"',"'"]:
                        warnings.warn("Basic Auth Realm was unquoted",
                                      UserWarning, 2)
                    if scheme.lower() == 'basic':
                        return self.retry_http_basic_auth(host, req, realm)

    def retry_http_basic_auth(self, host, req, realm):
        user, pw = self.passwd.find_user_password(realm, host)
        if pw is not None: