static inline unsigned int
get_rfc1002_length(void *buf)
{
	return be32_to_cpu(*((__be32 *)buf));
}

static inline void
inc_rfc1001_len(void *buf, int count)