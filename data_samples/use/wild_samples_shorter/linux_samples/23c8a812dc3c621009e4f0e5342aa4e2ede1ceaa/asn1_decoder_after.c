
	/* Extract a tag from the data */
	tag = data[dp++];
	if (tag == ASN1_EOC) {
		/* It appears to be an EOC. */
		if (data[dp++] != 0)
			goto invalid_eoc;
		if (--indef_level <= 0) {

	/* Extract the length */
	len = data[dp++];
	if (len <= 0x7f)
		goto check_length;

	if (unlikely(len == ASN1_INDEFINITE_LENGTH)) {
		/* Indefinite length */
		if (unlikely((tag & ASN1_CONS_BIT) == ASN1_PRIM << 5))
	}

	n = len - 0x80;
	if (unlikely(n > sizeof(len) - 1))
		goto length_too_long;
	if (unlikely(n > datalen - dp))
		goto data_overrun_error;
	len = 0;
	for (; n > 0; n--) {
		len <<= 8;
		len |= data[dp++];
	}
check_length:
	if (len > datalen - dp)
		goto data_overrun_error;
	dp += len;
	goto next_tag;

length_too_long: