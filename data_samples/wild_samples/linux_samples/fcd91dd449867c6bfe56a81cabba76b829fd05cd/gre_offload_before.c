			if (*(__be32 *)(greh2+1) != *(__be32 *)(greh+1)) {
				NAPI_GRO_CB(p)->same_flow = 0;
				continue;
			}
		}
	}

	skb_gro_pull(skb, grehlen);

	/* Adjusted NAPI_GRO_CB(skb)->csum after skb_gro_pull()*/
	skb_gro_postpull_rcsum(skb, greh, grehlen);

	pp = ptype->callbacks.gro_receive(head, skb);
	flush = 0;

out_unlock:
	rcu_read_unlock();
out:
	NAPI_GRO_CB(skb)->flush |= flush;

	return pp;
}

static int gre_gro_complete(struct sk_buff *skb, int nhoff)
{
	struct gre_base_hdr *greh = (struct gre_base_hdr *)(skb->data + nhoff);
	struct packet_offload *ptype;
	unsigned int grehlen = sizeof(*greh);
	int err = -ENOENT;
	__be16 type;

	skb->encapsulation = 1;
	skb_shinfo(skb)->gso_type = SKB_GSO_GRE;

	type = greh->protocol;
	if (greh->flags & GRE_KEY)
		grehlen += GRE_HEADER_SECTION;

	if (greh->flags & GRE_CSUM)
		grehlen += GRE_HEADER_SECTION;

	rcu_read_lock();
	ptype = gro_find_complete_by_type(type);
	if (ptype)
		err = ptype->callbacks.gro_complete(skb, nhoff + grehlen);

	rcu_read_unlock();

	skb_set_inner_mac_header(skb, nhoff + grehlen);

	return err;
}

static const struct net_offload gre_offload = {
	.callbacks = {
		.gso_segment = gre_gso_segment,
		.gro_receive = gre_gro_receive,
		.gro_complete = gre_gro_complete,
	},
};

static int __init gre_offload_init(void)
{
	int err;

	err = inet_add_offload(&gre_offload, IPPROTO_GRE);
#if IS_ENABLED(CONFIG_IPV6)
	if (err)
		return err;

	err = inet6_add_offload(&gre_offload, IPPROTO_GRE);
	if (err)
		inet_del_offload(&gre_offload, IPPROTO_GRE);
#endif

	return err;
}
device_initcall(gre_offload_init);