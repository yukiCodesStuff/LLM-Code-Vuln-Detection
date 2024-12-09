
	skb_gro_pull(skb, gh_len);
	skb_gro_postpull_rcsum(skb, gh, gh_len);
	pp = ptype->callbacks.gro_receive(head, skb);
	flush = 0;

out_unlock:
	rcu_read_unlock();