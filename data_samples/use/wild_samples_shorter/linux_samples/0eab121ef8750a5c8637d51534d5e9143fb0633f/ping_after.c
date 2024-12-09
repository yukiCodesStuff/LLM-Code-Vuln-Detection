	if (len > 0xFFFF)
		return -EMSGSIZE;

	/* Must have at least a full ICMP header. */
	if (len < icmph_len)
		return -EINVAL;

	/*
	 *	Check the flags.
	 */
