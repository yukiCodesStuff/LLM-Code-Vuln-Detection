{
	__trace_seq_init(s);
	return seq_buf_to_user(&s->seq, ubuf, cnt);
}
EXPORT_SYMBOL_GPL(trace_seq_to_user);

int trace_seq_hex_dump(struct trace_seq *s, const char *prefix_str,
		       int prefix_type, int rowsize, int groupsize,
		       const void *buf, size_t len, bool ascii)
{
		unsigned int save_len = s->seq.len;

	if (s->full)
		return 0;

	__trace_seq_init(s);

	if (TRACE_SEQ_BUF_LEFT(s) < 1) {
		s->full = 1;
		return 0;
	}

	seq_buf_hex_dump(&(s->seq), prefix_str,
		   prefix_type, rowsize, groupsize,
		   buf, len, ascii);

	if (unlikely(seq_buf_has_overflowed(&s->seq))) {
		s->seq.len = save_len;
		s->full = 1;
		return 0;
	}

	return 1;
}