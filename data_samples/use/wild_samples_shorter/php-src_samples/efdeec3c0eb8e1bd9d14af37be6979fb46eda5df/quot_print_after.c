	unsigned char c, *ret, *d;
	char *hex = "0123456789ABCDEF";

	ret = safe_emalloc(3, length + (((3 * length)/(PHP_QPRINT_MAXL-9)) + 1), 1);
	d = ret;

	while (length--) {
		if (((c = *str++) == '\015') && (*str == '\012') && length > 0) {
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */