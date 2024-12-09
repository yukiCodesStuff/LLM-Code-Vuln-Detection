	spin_unlock(&f->lock);
}

static int fanout_add(struct sock *sk, u16 id, u16 type_flags)
{
	struct packet_sock *po = pkt_sk(sk);
	struct packet_fanout *f, *match;
		match->prot_hook.dev = po->prot_hook.dev;
		match->prot_hook.func = packet_rcv_fanout;
		match->prot_hook.af_packet_priv = match;
		dev_add_pack(&match->prot_hook);
		list_add(&match->list, &fanout_list);
	}
	err = -EINVAL;