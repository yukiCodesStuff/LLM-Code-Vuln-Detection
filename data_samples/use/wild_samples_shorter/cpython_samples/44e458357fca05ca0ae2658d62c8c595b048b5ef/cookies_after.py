        return '"' + str.translate(_Translator) + '"'


_unquote_sub = re.compile(r'\\(?:([0-3][0-7][0-7])|(.))').sub

def _unquote_replace(m):
    if m[1]:
        return chr(int(m[1], 8))
    else:
        return m[2]

def _unquote(str):
    # If there aren't any doublequotes,
    # then there can't be any special characters.  See RFC 2109.
    #    \012 --> \n
    #    \"   --> "
    #
    return _unquote_sub(_unquote_replace, str)

# The _getdate() routine is used to set the expiration time in the cookie's HTTP
# header.  By default, _getdate() returns the current time in the appropriate
# "expires" format for a Set-Cookie header.  The one optional argument is an