				sk_no_check_rx : 1,
				sk_userlocks : 4,
				sk_protocol  : 8,
#define SK_PROTOCOL_MAX U8_MAX
				sk_type      : 16;
	kmemcheck_bitfield_end(flags);
	int			sk_wmem_queued;
	gfp_t			sk_allocation;