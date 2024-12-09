	unsigned long limit;
	unsigned int i;

	BUILD_BUG_ON(TCP_MIN_SND_MSS <= MAX_TCP_OPTION_SPACE);
	BUILD_BUG_ON(sizeof(struct tcp_skb_cb) >
		     FIELD_SIZEOF(struct sk_buff, cb));

	percpu_counter_init(&tcp_sockets_allocated, 0, GFP_KERNEL);