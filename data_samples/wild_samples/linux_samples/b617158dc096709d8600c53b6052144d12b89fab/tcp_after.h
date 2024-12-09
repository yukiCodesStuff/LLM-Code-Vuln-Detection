{
	return skb_rb_first(&sk->tcp_rtx_queue);
}

static inline struct sk_buff *tcp_rtx_queue_tail(const struct sock *sk)
{
	return skb_rb_last(&sk->tcp_rtx_queue);
}

static inline struct sk_buff *tcp_write_queue_head(const struct sock *sk)
{
	return skb_peek(&sk->sk_write_queue);
}

static inline struct sk_buff *tcp_write_queue_tail(const struct sock *sk)
{
	return skb_peek_tail(&sk->sk_write_queue);
}

#define tcp_for_write_queue_from_safe(skb, tmp, sk)			\
	skb_queue_walk_from_safe(&(sk)->sk_write_queue, skb, tmp)

static inline struct sk_buff *tcp_send_head(const struct sock *sk)
{
	return skb_peek(&sk->sk_write_queue);
}

static inline bool tcp_skb_is_last(const struct sock *sk,
				   const struct sk_buff *skb)
{
	return skb_queue_is_last(&sk->sk_write_queue, skb);
}

static inline bool tcp_write_queue_empty(const struct sock *sk)
{
	return skb_queue_empty(&sk->sk_write_queue);
}

static inline bool tcp_rtx_queue_empty(const struct sock *sk)
{
	return RB_EMPTY_ROOT(&sk->tcp_rtx_queue);
}

static inline bool tcp_rtx_and_write_queues_empty(const struct sock *sk)
{
	return tcp_rtx_queue_empty(sk) && tcp_write_queue_empty(sk);
}

static inline void tcp_add_write_queue_tail(struct sock *sk, struct sk_buff *skb)
{
	__skb_queue_tail(&sk->sk_write_queue, skb);

	/* Queue it, remembering where we must start sending. */
	if (sk->sk_write_queue.next == skb)
		tcp_chrono_start(sk, TCP_CHRONO_BUSY);
}

/* Insert new before skb on the write queue of sk.  */
static inline void tcp_insert_write_queue_before(struct sk_buff *new,
						  struct sk_buff *skb,
						  struct sock *sk)
{
	__skb_queue_before(&sk->sk_write_queue, skb, new);
}

static inline void tcp_unlink_write_queue(struct sk_buff *skb, struct sock *sk)
{
	tcp_skb_tsorted_anchor_cleanup(skb);
	__skb_unlink(skb, &sk->sk_write_queue);
}

void tcp_rbtree_insert(struct rb_root *root, struct sk_buff *skb);

static inline void tcp_rtx_queue_unlink(struct sk_buff *skb, struct sock *sk)
{
	tcp_skb_tsorted_anchor_cleanup(skb);
	rb_erase(&skb->rbnode, &sk->tcp_rtx_queue);
}

static inline void tcp_rtx_queue_unlink_and_free(struct sk_buff *skb, struct sock *sk)
{
	list_del(&skb->tcp_tsorted_anchor);
	tcp_rtx_queue_unlink(skb, sk);
	sk_wmem_free_skb(sk, skb);
}

static inline void tcp_push_pending_frames(struct sock *sk)
{
	if (tcp_send_head(sk)) {
		struct tcp_sock *tp = tcp_sk(sk);

		__tcp_push_pending_frames(sk, tcp_current_mss(sk), tp->nonagle);
	}