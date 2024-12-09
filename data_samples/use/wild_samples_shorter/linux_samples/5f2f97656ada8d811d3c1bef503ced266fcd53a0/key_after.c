				       unsigned int *_toklen)
{
	const __be32 *xdr = *_xdr;
	unsigned int toklen = *_toklen, n_parts, loop, tmp, paddedlen;

	/* there must be at least one name, and at least #names+1 length
	 * words */
	if (toklen <= 12)
		toklen -= 4;
		if (tmp <= 0 || tmp > AFSTOKEN_STRING_MAX)
			return -EINVAL;
		paddedlen = (tmp + 3) & ~3;
		if (paddedlen > toklen)
			return -EINVAL;
		princ->name_parts[loop] = kmalloc(tmp + 1, GFP_KERNEL);
		if (!princ->name_parts[loop])
			return -ENOMEM;
		memcpy(princ->name_parts[loop], xdr, tmp);
		princ->name_parts[loop][tmp] = 0;
		toklen -= paddedlen;
		xdr += paddedlen >> 2;
	}

	if (toklen < 4)
		return -EINVAL;
	toklen -= 4;
	if (tmp <= 0 || tmp > AFSTOKEN_K5_REALM_MAX)
		return -EINVAL;
	paddedlen = (tmp + 3) & ~3;
	if (paddedlen > toklen)
		return -EINVAL;
	princ->realm = kmalloc(tmp + 1, GFP_KERNEL);
	if (!princ->realm)
		return -ENOMEM;
	memcpy(princ->realm, xdr, tmp);
	princ->realm[tmp] = 0;
	toklen -= paddedlen;
	xdr += paddedlen >> 2;

	_debug("%s/...@%s", princ->name_parts[0], princ->realm);

	*_xdr = xdr;
					 unsigned int *_toklen)
{
	const __be32 *xdr = *_xdr;
	unsigned int toklen = *_toklen, len, paddedlen;

	/* there must be at least one tag and one length word */
	if (toklen <= 8)
		return -EINVAL;
	toklen -= 8;
	if (len > max_data_size)
		return -EINVAL;
	paddedlen = (len + 3) & ~3;
	if (paddedlen > toklen)
		return -EINVAL;
	td->data_len = len;

	if (len > 0) {
		td->data = kmemdup(xdr, len, GFP_KERNEL);
		if (!td->data)
			return -ENOMEM;
		toklen -= paddedlen;
		xdr += paddedlen >> 2;
	}

	_debug("tag %x len %x", td->tag, td->data_len);

				    const __be32 **_xdr, unsigned int *_toklen)
{
	const __be32 *xdr = *_xdr;
	unsigned int toklen = *_toklen, len, paddedlen;

	/* there must be at least one length word */
	if (toklen <= 4)
		return -EINVAL;
	toklen -= 4;
	if (len > AFSTOKEN_K5_TIX_MAX)
		return -EINVAL;
	paddedlen = (len + 3) & ~3;
	if (paddedlen > toklen)
		return -EINVAL;
	*_tktlen = len;

	_debug("ticket len %u", len);

		*_ticket = kmemdup(xdr, len, GFP_KERNEL);
		if (!*_ticket)
			return -ENOMEM;
		toklen -= paddedlen;
		xdr += paddedlen >> 2;
	}

	*_xdr = xdr;
	*_toklen = toklen;
{
	const __be32 *xdr = prep->data, *token;
	const char *cp;
	unsigned int len, paddedlen, loop, ntoken, toklen, sec_ix;
	size_t datalen = prep->datalen;
	int ret;

	_enter(",{%x,%x,%x,%x},%zu",
	if (len < 1 || len > AFSTOKEN_CELL_MAX)
		goto not_xdr;
	datalen -= 4;
	paddedlen = (len + 3) & ~3;
	if (paddedlen > datalen)
		goto not_xdr;

	cp = (const char *) xdr;
	for (loop = 0; loop < len; loop++)
		if (!isprint(cp[loop]))
			goto not_xdr;
	for (; loop < paddedlen; loop++)
		if (cp[loop])
			goto not_xdr;
	_debug("cellname: [%u/%u] '%*.*s'",
	       len, paddedlen, len, len, (const char *) xdr);
	datalen -= paddedlen;
	xdr += paddedlen >> 2;

	/* get the token count */
	if (datalen < 12)
		goto not_xdr;
		sec_ix = ntohl(*xdr);
		datalen -= 4;
		_debug("token: [%x/%zx] %x", toklen, datalen, sec_ix);
		paddedlen = (toklen + 3) & ~3;
		if (toklen < 20 || toklen > datalen || paddedlen > datalen)
			goto not_xdr;
		datalen -= paddedlen;
		xdr += paddedlen >> 2;

	} while (--loop > 0);

	_debug("remainder: %zu", datalen);