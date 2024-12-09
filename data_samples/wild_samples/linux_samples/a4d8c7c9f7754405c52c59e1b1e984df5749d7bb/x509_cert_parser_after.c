	} else {
		goto unsupported_time;
	}

	mon  = DD2bin(p);
	day = DD2bin(p);
	hour = DD2bin(p);
	min  = DD2bin(p);
	sec  = DD2bin(p);

	if (*p != 'Z')
		goto unsupported_time;

	if (year < 1970 ||
	    mon < 1 || mon > 12)
		goto invalid_time;

	mon_len = month_lengths[mon - 1];
	if (mon == 2) {
		if (year % 4 == 0) {
			mon_len = 29;
			if (year % 100 == 0) {
				year /= 100;
				if (year % 4 != 0)
					mon_len = 28;
			}
		}
	}
			if (year % 100 == 0) {
				year /= 100;
				if (year % 4 != 0)
					mon_len = 28;
			}
		}
	}

	if (day < 1 || day > mon_len ||
	    hour > 23 ||
	    min > 59 ||
	    sec > 59)
		goto invalid_time;

	*_t = mktime64(year, mon, day, hour, min, sec);
	return 0;

unsupported_time:
	pr_debug("Got unsupported time [tag %02x]: '%*phN'\n",
		 tag, (int)vlen, value);
	return -EBADMSG;
invalid_time:
	pr_debug("Got invalid time [tag %02x]: '%*phN'\n",
		 tag, (int)vlen, value);
	return -EBADMSG;
}
EXPORT_SYMBOL_GPL(x509_decode_time);

int x509_note_not_before(void *context, size_t hdrlen,
			 unsigned char tag,
			 const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;
	return x509_decode_time(&ctx->cert->valid_from, hdrlen, tag, value, vlen);
}

int x509_note_not_after(void *context, size_t hdrlen,
			unsigned char tag,
			const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;
	return x509_decode_time(&ctx->cert->valid_to, hdrlen, tag, value, vlen);
}

/*
 * Note a key identifier-based AuthorityKeyIdentifier
 */
int x509_akid_note_kid(void *context, size_t hdrlen,
		       unsigned char tag,
		       const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;
	struct asymmetric_key_id *kid;

	pr_debug("AKID: keyid: %*phN\n", (int)vlen, value);

	if (ctx->cert->akid_skid)
		return 0;

	kid = asymmetric_key_generate_id(value, vlen, "", 0);
	if (IS_ERR(kid))
		return PTR_ERR(kid);
	pr_debug("authkeyid %*phN\n", kid->len, kid->data);
	ctx->cert->akid_skid = kid;
	return 0;
}

/*
 * Note a directoryName in an AuthorityKeyIdentifier
 */
int x509_akid_note_name(void *context, size_t hdrlen,
			unsigned char tag,
			const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;

	pr_debug("AKID: name: %*phN\n", (int)vlen, value);

	ctx->akid_raw_issuer = value;
	ctx->akid_raw_issuer_size = vlen;
	return 0;
}

/*
 * Note a serial number in an AuthorityKeyIdentifier
 */
int x509_akid_note_serial(void *context, size_t hdrlen,
			  unsigned char tag,
			  const void *value, size_t vlen)
{
	struct x509_parse_context *ctx = context;
	struct asymmetric_key_id *kid;

	pr_debug("AKID: serial: %*phN\n", (int)vlen, value);

	if (!ctx->akid_raw_issuer || ctx->cert->akid_id)
		return 0;

	kid = asymmetric_key_generate_id(value,
					 vlen,
					 ctx->akid_raw_issuer,
					 ctx->akid_raw_issuer_size);
	if (IS_ERR(kid))
		return PTR_ERR(kid);

	pr_debug("authkeyid %*phN\n", kid->len, kid->data);
	ctx->cert->akid_id = kid;
	return 0;
}