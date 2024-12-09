	unsigned char c, *ret, *d;
	char *hex = "0123456789ABCDEF";

	ret = safe_emalloc(1, 3 * length + 3 * (((3 * length)/PHP_QPRINT_MAXL) + 1), 0);
	d = ret;

	while (length--) {
		if (((c = *str++) == '\015') && (*str == '\012') && length > 0) {