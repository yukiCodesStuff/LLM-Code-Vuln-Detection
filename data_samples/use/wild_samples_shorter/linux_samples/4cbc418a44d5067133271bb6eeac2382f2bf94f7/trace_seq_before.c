		       int prefix_type, int rowsize, int groupsize,
		       const void *buf, size_t len, bool ascii)
{
		unsigned int save_len = s->seq.len;

	if (s->full)
		return 0;
