	char *p, *q;
	int chunks; /* complete chunks! */
	int restlen;
	int out_len; 

	chunks = srclen / chunklen;
	restlen = srclen - chunks * chunklen; /* srclen % chunklen */

	out_len = (srclen + (chunks + 1) * endlen + 1);

	if (out_len > INT_MAX || out_len <= 0) {
		return NULL;
	}

	dest = safe_emalloc(out_len, sizeof(char), 0);

	for (p = src, q = dest; p < (src + srclen - chunklen + 1); ) {
		memcpy(q, p, chunklen);
		q += chunklen;