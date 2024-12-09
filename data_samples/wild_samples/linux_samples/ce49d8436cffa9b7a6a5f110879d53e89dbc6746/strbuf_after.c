	if (!strbuf_avail(sb)) {
		ret = strbuf_grow(sb, 64);
		if (ret)
			return ret;
	}

	va_copy(ap_saved, ap);
	len = vsnprintf(sb->buf + sb->len, sb->alloc - sb->len, fmt, ap);
	if (len < 0) {
		va_end(ap_saved);
		return len;
	}