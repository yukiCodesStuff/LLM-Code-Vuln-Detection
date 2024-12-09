
	skb_gro_pull(skb, sizeof(*vhdr));
	skb_gro_postpull_rcsum(skb, vhdr, sizeof(*vhdr));
	pp = call_gro_receive(ptype->callbacks.gro_receive, head, skb);

out_unlock:
	rcu_read_unlock();
out: