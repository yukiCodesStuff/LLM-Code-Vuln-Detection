void llc_add_pack(int type, void (*handler)(struct llc_sap *sap,
					    struct sk_buff *skb))
{
	smp_wmb(); /* ensure initialisation is complete before it's called */
	if (type == LLC_DEST_SAP || type == LLC_DEST_CONN)
		llc_type_handlers[type - 1] = handler;
}

{
	if (type == LLC_DEST_SAP || type == LLC_DEST_CONN)
		llc_type_handlers[type - 1] = NULL;
	synchronize_net();
}

void llc_set_station_handler(void (*handler)(struct sk_buff *skb))
{
	/* Ensure initialisation is complete before it's called */
	if (handler)
		smp_wmb();

	llc_station_handler = handler;

	if (!handler)
		synchronize_net();
}

/**
 *	llc_pdu_type - returns which LLC component must handle for PDU
	int dest;
	int (*rcv)(struct sk_buff *, struct net_device *,
		   struct packet_type *, struct net_device *);
	void (*sta_handler)(struct sk_buff *skb);
	void (*sap_handler)(struct llc_sap *sap, struct sk_buff *skb);

	if (!net_eq(dev_net(dev), &init_net))
		goto drop;

	 */
	rcv = rcu_dereference(sap->rcv_func);
	dest = llc_pdu_type(skb);
	sap_handler = dest ? ACCESS_ONCE(llc_type_handlers[dest - 1]) : NULL;
	if (unlikely(!sap_handler)) {
		if (rcv)
			rcv(skb, dev, pt, orig_dev);
		else
			kfree_skb(skb);
			if (cskb)
				rcv(cskb, dev, pt, orig_dev);
		}
		sap_handler(sap, skb);
	}
	llc_sap_put(sap);
out:
	return 0;
	kfree_skb(skb);
	goto out;
handle_station:
	sta_handler = ACCESS_ONCE(llc_station_handler);
	if (!sta_handler)
		goto drop;
	sta_handler(skb);
	goto out;
}

EXPORT_SYMBOL(llc_add_pack);