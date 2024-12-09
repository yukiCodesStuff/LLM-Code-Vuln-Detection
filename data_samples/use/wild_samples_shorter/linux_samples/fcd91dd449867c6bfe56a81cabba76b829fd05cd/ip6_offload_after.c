
	skb_gro_postpull_rcsum(skb, iph, nlen);

	pp = call_gro_receive(ops->callbacks.gro_receive, head, skb);

out_unlock:
	rcu_read_unlock();
