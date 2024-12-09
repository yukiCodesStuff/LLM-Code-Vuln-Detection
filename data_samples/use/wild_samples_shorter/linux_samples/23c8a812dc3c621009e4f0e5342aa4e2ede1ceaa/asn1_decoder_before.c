
	/* Extract a tag from the data */
	tag = data[dp++];
	if (tag == 0) {
		/* It appears to be an EOC. */
		if (data[dp++] != 0)
			goto invalid_eoc;
		if (--indef_level <= 0) {

	/* Extract the length */
	len = data[dp++];
	if (len <= 0x7f) {
		dp += len;
		goto next_tag;
	}

	if (unlikely(len == ASN1_INDEFINITE_LENGTH)) {
		/* Indefinite length */
		if (unlikely((tag & ASN1_CONS_BIT) == ASN1_PRIM << 5))
	}

	n = len - 0x80;
	if (unlikely(n > sizeof(size_t) - 1))
		goto length_too_long;
	if (unlikely(n > datalen - dp))
		goto data_overrun_error;
	for (len = 0; n > 0; n--) {
		len <<= 8;
		len |= data[dp++];
	}
	dp += len;
	goto next_tag;

length_too_long: