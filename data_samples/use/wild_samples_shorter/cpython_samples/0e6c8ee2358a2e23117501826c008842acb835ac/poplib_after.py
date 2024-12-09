        return self._shortcmd('RPOP %s' % user)


    timestamp = re.compile(br'\+OK.[^<]*(<.*>)')

    def apop(self, user, password):
        """Authorisation
