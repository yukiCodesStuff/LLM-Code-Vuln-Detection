	return skb_rb_first(&sk->tcp_rtx_queue);
}

static inline struct sk_buff *tcp_write_queue_head(const struct sock *sk)
{
	return skb_peek(&sk->sk_write_queue);
}