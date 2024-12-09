	do {
		flags = tp->tsq_flags;
		if (!(flags & TCP_DEFERRED_ALL))
			return;
		nflags = flags & ~TCP_DEFERRED_ALL;
	} while (cmpxchg(&tp->tsq_flags, flags, nflags) != flags);

	if (flags & (1UL << TCP_TSQ_DEFERRED))
		tcp_tsq_handler(sk);

	if (flags & (1UL << TCP_WRITE_TIMER_DEFERRED))
		tcp_write_timer_handler(sk);

	if (flags & (1UL << TCP_DELACK_TIMER_DEFERRED))
		tcp_delack_timer_handler(sk);

	if (flags & (1UL << TCP_MTU_REDUCED_DEFERRED))
		sk->sk_prot->mtu_reduced(sk);
}
EXPORT_SYMBOL(tcp_release_cb);

void __init tcp_tasklet_init(void)
{
	int i;

	for_each_possible_cpu(i) {
		struct tsq_tasklet *tsq = &per_cpu(tsq_tasklet, i);

		INIT_LIST_HEAD(&tsq->head);
		tasklet_init(&tsq->tasklet,
			     tcp_tasklet_func,
			     (unsigned long)tsq);
	}