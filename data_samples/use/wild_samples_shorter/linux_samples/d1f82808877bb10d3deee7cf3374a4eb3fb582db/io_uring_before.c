struct io_buffer {
	struct list_head list;
	__u64 addr;
	__s32 len;
	__u16 bid;
};

struct io_restriction {
			break;

		buf->addr = addr;
		buf->len = pbuf->len;
		buf->bid = bid;
		addr += pbuf->len;
		bid++;
		if (!*head) {