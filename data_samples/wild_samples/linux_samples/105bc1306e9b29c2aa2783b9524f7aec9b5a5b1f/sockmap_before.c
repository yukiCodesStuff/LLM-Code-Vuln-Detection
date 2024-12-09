	while (sg[i].length) {
		free += sg[i].length;
		if (charge)
			sk_mem_uncharge(sk, sg[i].length);
		if (!md->skb)
			put_page(sg_page(&sg[i]));
		sg[i].length = 0;
		sg[i].page_link = 0;
		sg[i].offset = 0;
		i++;

		if (i == MAX_SKB_FRAGS)
			i = 0;
	}
	if (md->skb)
		consume_skb(md->skb);

	return free;
}

static int free_start_sg(struct sock *sk, struct sk_msg_buff *md, bool charge)
{
	int free = free_sg(sk, md->sg_start, md, charge);

	md->sg_start = md->sg_end;
	return free;
}

static int free_curr_sg(struct sock *sk, struct sk_msg_buff *md)
{
	return free_sg(sk, md->sg_curr, md, true);
}

static int bpf_map_msg_verdict(int _rc, struct sk_msg_buff *md)
{
	return ((_rc == SK_PASS) ?
	       (md->sk_redir ? __SK_REDIRECT : __SK_PASS) :
	       __SK_DROP);
}

static unsigned int smap_do_tx_msg(struct sock *sk,
				   struct smap_psock *psock,
				   struct sk_msg_buff *md)
{
	struct bpf_prog *prog;
	unsigned int rc, _rc;

	preempt_disable();
	rcu_read_lock();

	/* If the policy was removed mid-send then default to 'accept' */
	prog = READ_ONCE(psock->bpf_tx_msg);
	if (unlikely(!prog)) {
		_rc = SK_PASS;
		goto verdict;
	}

	bpf_compute_data_pointers_sg(md);
	md->sk = sk;
	rc = (*prog->bpf_func)(md, prog->insnsi);
	psock->apply_bytes = md->apply_bytes;

	/* Moving return codes from UAPI namespace into internal namespace */
	_rc = bpf_map_msg_verdict(rc, md);

	/* The psock has a refcount on the sock but not on the map and because
	 * we need to drop rcu read lock here its possible the map could be
	 * removed between here and when we need it to execute the sock
	 * redirect. So do the map lookup now for future use.
	 */
	if (_rc == __SK_REDIRECT) {
		if (psock->sk_redir)
			sock_put(psock->sk_redir);
		psock->sk_redir = do_msg_redirect_map(md);
		if (!psock->sk_redir) {
			_rc = __SK_DROP;
			goto verdict;
		}
		if (!sg->length && md->sg_start == md->sg_end) {
			list_del(&md->list);
			if (md->skb)
				consume_skb(md->skb);
			kfree(md);
		}