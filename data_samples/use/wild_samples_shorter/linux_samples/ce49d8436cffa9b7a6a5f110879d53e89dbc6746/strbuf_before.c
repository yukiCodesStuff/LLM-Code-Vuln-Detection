
	va_copy(ap_saved, ap);
	len = vsnprintf(sb->buf + sb->len, sb->alloc - sb->len, fmt, ap);
	if (len < 0)
		return len;
	if (len > strbuf_avail(sb)) {
		ret = strbuf_grow(sb, len);
		if (ret)
			return ret;
		len = vsnprintf(sb->buf + sb->len, sb->alloc - sb->len, fmt, ap_saved);
		va_end(ap_saved);
		if (len > strbuf_avail(sb)) {
			pr_debug("this should not happen, your vsnprintf is broken");
			return -EINVAL;
		}
	}
	return strbuf_setlen(sb, sb->len + len);
}

int strbuf_addf(struct strbuf *sb, const char *fmt, ...)