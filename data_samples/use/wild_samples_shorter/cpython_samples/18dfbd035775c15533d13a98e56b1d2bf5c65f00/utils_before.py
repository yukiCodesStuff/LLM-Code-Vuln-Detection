    return address



def getaddresses(fieldvalues):
    """Return a list of (REALNAME, EMAIL) for each fieldvalue."""
    all = COMMASPACE.join(str(v) for v in fieldvalues)
    a = _AddressList(all)
    return a.addresslist


def _format_timetuple_and_zone(timetuple, zone):
    return '%s, %02d %s %04d %02d:%02d:%02d %s' % (
    Return a tuple of realname and email address, unless the parse fails, in
    which case return a 2-tuple of ('', '').
    """
    addrs = _AddressList(addr).addresslist
    if not addrs:
        return '', ''
    return addrs[0]


# rfc822.unquote() doesn't properly de-backslash-ify in Python pre-2.3.