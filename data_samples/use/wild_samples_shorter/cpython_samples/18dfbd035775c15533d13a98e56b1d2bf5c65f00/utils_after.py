    return address


def _pre_parse_validation(email_header_fields):
    accepted_values = []
    for v in email_header_fields:
        s = v.replace('\\(', '').replace('\\)', '')
        if s.count('(') != s.count(')'):
            v = "('', '')"
        accepted_values.append(v)

    return accepted_values


def _post_parse_validation(parsed_email_header_tuples):
    accepted_values = []
    # The parser would have parsed a correctly formatted domain-literal
    # The existence of an [ after parsing indicates a parsing failure
    for v in parsed_email_header_tuples:
        if '[' in v[1]:
            v = ('', '')
        accepted_values.append(v)

    return accepted_values


def getaddresses(fieldvalues):
    """Return a list of (REALNAME, EMAIL) or ('','') for each fieldvalue.

    When parsing fails for a fieldvalue, a 2-tuple of ('', '') is returned in
    its place.

    If the resulting list of parsed address is not the same as the number of
    fieldvalues in the input list a parsing error has occurred.  A list
    containing a single empty 2-tuple [('', '')] is returned in its place.
    This is done to avoid invalid output.
    """
    fieldvalues = [str(v) for v in fieldvalues]
    fieldvalues = _pre_parse_validation(fieldvalues)
    all = COMMASPACE.join(v for v in fieldvalues)
    a = _AddressList(all)
    result = _post_parse_validation(a.addresslist)

    n = 0
    for v in fieldvalues:
        n += v.count(',') + 1

    if len(result) != n:
        return [('', '')]

    return result


def _format_timetuple_and_zone(timetuple, zone):
    return '%s, %02d %s %04d %02d:%02d:%02d %s' % (
    Return a tuple of realname and email address, unless the parse fails, in
    which case return a 2-tuple of ('', '').
    """
    if isinstance(addr, list):
        addr = addr[0]

    if not isinstance(addr, str):
        return ('', '')

    addr = _pre_parse_validation([addr])[0]
    addrs = _post_parse_validation(_AddressList(addr).addresslist)

    if not addrs or len(addrs) > 1:
        return ('', '')

    return addrs[0]


# rfc822.unquote() doesn't properly de-backslash-ify in Python pre-2.3.