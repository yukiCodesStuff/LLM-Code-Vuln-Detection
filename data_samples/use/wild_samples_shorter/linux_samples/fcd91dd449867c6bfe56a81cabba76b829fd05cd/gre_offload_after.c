	/* Adjusted NAPI_GRO_CB(skb)->csum after skb_gro_pull()*/
	skb_gro_postpull_rcsum(skb, greh, grehlen);

	pp = call_gro_receive(ptype->callbacks.gro_receive, head, skb);
	flush = 0;

out_unlock:
	rcu_read_unlock();