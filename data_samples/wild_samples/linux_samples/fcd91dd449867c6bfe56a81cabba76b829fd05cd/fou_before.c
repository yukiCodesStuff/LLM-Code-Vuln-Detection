{
	const struct net_offload *ops;
	struct sk_buff **pp = NULL;
	u8 proto = fou_from_sock(sk)->protocol;
	const struct net_offload **offloads;

	/* We can clear the encap_mark for FOU as we are essentially doing
	 * one of two possible things.  We are either adding an L4 tunnel
	 * header to the outer L3 tunnel header, or we are are simply
	 * treating the GRE tunnel header as though it is a UDP protocol
	 * specific header such as VXLAN or GENEVE.
	 */
	NAPI_GRO_CB(skb)->encap_mark = 0;

	/* Flag this frame as already having an outer encap header */
	NAPI_GRO_CB(skb)->is_fou = 1;

	rcu_read_lock();
	offloads = NAPI_GRO_CB(skb)->is_ipv6 ? inet6_offloads : inet_offloads;
	ops = rcu_dereference(offloads[proto]);
	if (!ops || !ops->callbacks.gro_receive)
		goto out_unlock;

	pp = ops->callbacks.gro_receive(head, skb);

out_unlock:
	rcu_read_unlock();

	return pp;
}

static int fou_gro_complete(struct sock *sk, struct sk_buff *skb,
			    int nhoff)
{
	const struct net_offload *ops;
	u8 proto = fou_from_sock(sk)->protocol;
	int err = -ENOSYS;
	const struct net_offload **offloads;

	rcu_read_lock();
	offloads = NAPI_GRO_CB(skb)->is_ipv6 ? inet6_offloads : inet_offloads;
	ops = rcu_dereference(offloads[proto]);
	if (WARN_ON(!ops || !ops->callbacks.gro_complete))
		goto out_unlock;

	err = ops->callbacks.gro_complete(skb, nhoff);

	skb_set_inner_mac_header(skb, nhoff);

out_unlock:
	rcu_read_unlock();

	return err;
}

static struct guehdr *gue_gro_remcsum(struct sk_buff *skb, unsigned int off,
				      struct guehdr *guehdr, void *data,
				      size_t hdrlen, struct gro_remcsum *grc,
				      bool nopartial)
{
	__be16 *pd = data;
	size_t start = ntohs(pd[0]);
	size_t offset = ntohs(pd[1]);

	if (skb->remcsum_offload)
		return guehdr;

	if (!NAPI_GRO_CB(skb)->csum_valid)
		return NULL;

	guehdr = skb_gro_remcsum_process(skb, (void *)guehdr, off, hdrlen,
					 start, offset, grc, nopartial);

	skb->remcsum_offload = 1;

	return guehdr;
}

static struct sk_buff **gue_gro_receive(struct sock *sk,
					struct sk_buff **head,
					struct sk_buff *skb)
{
	const struct net_offload **offloads;
	const struct net_offload *ops;
	struct sk_buff **pp = NULL;
	struct sk_buff *p;
	struct guehdr *guehdr;
	size_t len, optlen, hdrlen, off;
	void *data;
	u16 doffset = 0;
	int flush = 1;
	struct fou *fou = fou_from_sock(sk);
	struct gro_remcsum grc;
	u8 proto;

	skb_gro_remcsum_init(&grc);

	off = skb_gro_offset(skb);
	len = off + sizeof(*guehdr);

	guehdr = skb_gro_header_fast(skb, off);
	if (skb_gro_header_hard(skb, len)) {
		guehdr = skb_gro_header_slow(skb, len, off);
		if (unlikely(!guehdr))
			goto out;
	}
					   guehdr->hlen << 2)) {
			NAPI_GRO_CB(p)->same_flow = 0;
			continue;
		}
	}

	proto = guehdr->proto_ctype;

next_proto:

	/* We can clear the encap_mark for GUE as we are essentially doing
	 * one of two possible things.  We are either adding an L4 tunnel
	 * header to the outer L3 tunnel header, or we are are simply
	 * treating the GRE tunnel header as though it is a UDP protocol
	 * specific header such as VXLAN or GENEVE.
	 */
	NAPI_GRO_CB(skb)->encap_mark = 0;

	/* Flag this frame as already having an outer encap header */
	NAPI_GRO_CB(skb)->is_fou = 1;

	rcu_read_lock();
	offloads = NAPI_GRO_CB(skb)->is_ipv6 ? inet6_offloads : inet_offloads;
	ops = rcu_dereference(offloads[proto]);
	if (WARN_ON_ONCE(!ops || !ops->callbacks.gro_receive))
		goto out_unlock;

	pp = ops->callbacks.gro_receive(head, skb);
	flush = 0;

out_unlock:
	rcu_read_unlock();
out:
	NAPI_GRO_CB(skb)->flush |= flush;
	skb_gro_remcsum_cleanup(skb, &grc);

	return pp;
}

static int gue_gro_complete(struct sock *sk, struct sk_buff *skb, int nhoff)
{
	const struct net_offload **offloads;
	struct guehdr *guehdr = (struct guehdr *)(skb->data + nhoff);
	const struct net_offload *ops;
	unsigned int guehlen = 0;
	u8 proto;
	int err = -ENOENT;

	switch (guehdr->version) {
	case 0:
		proto = guehdr->proto_ctype;
		guehlen = sizeof(*guehdr) + (guehdr->hlen << 2);
		break;
	case 1:
		switch (((struct iphdr *)guehdr)->version) {
		case 4:
			proto = IPPROTO_IPIP;
			break;
		case 6:
			proto = IPPROTO_IPV6;
			break;
		default:
			return err;
		}
		break;
	default:
		return err;
	}