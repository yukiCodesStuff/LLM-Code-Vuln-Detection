	int chunks; /* complete chunks! */
	int restlen;
	int charsize = sizeof(char);
	int out_len;

	if (str_type == IS_UNICODE) {
		charsize = sizeof(UChar);
	}
	chunks = srclen / chunklen;
	restlen = srclen - chunks * chunklen; /* srclen % chunklen */

	out_len = (srclen + (chunks + 1) * endlen + 1);

	if ((out_len > INT_MAX || out_len <= 0) || ((out_len * charsize) > INT_MAX || (out_len * charsize) <= 0)) {
		return NULL;
	}

	dest = safe_emalloc(out_len, charsize, 0);

	for (p = src, q = dest; p < (src + charsize * (srclen - chunklen + 1)); ) {
		memcpy(q, p, chunklen * charsize);
		q += chunklen * charsize;