struct io_buffer {
	struct list_head list;
	__u64 addr;
	__u32 len;
	__u16 bid;
};

struct io_restriction {
			break;

		buf->addr = addr;
		buf->len = min_t(__u32, pbuf->len, MAX_RW_COUNT);
		buf->bid = bid;
		addr += pbuf->len;
		bid++;
		if (!*head) {