
	skb_gro_pull(skb, sizeof(struct udphdr)); /* pull encapsulating udp header */
	skb_gro_postpull_rcsum(skb, uh, sizeof(struct udphdr));
	pp = udp_sk(sk)->gro_receive(sk, head, skb);

out_unlock:
	rcu_read_unlock();
out: