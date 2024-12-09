	skb_gro_pull(skb, sizeof(*iph));
	skb_set_transport_header(skb, skb_gro_offset(skb));

	pp = ops->callbacks.gro_receive(head, skb);

out_unlock:
	rcu_read_unlock();
