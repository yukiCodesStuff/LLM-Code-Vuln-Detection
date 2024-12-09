void llc_add_pack(int type, void (*handler)(struct llc_sap *sap,
					    struct sk_buff *skb))
{
	if (type == LLC_DEST_SAP || type == LLC_DEST_CONN)
		llc_type_handlers[type - 1] = handler;
}

{
	if (type == LLC_DEST_SAP || type == LLC_DEST_CONN)
		llc_type_handlers[type - 1] = NULL;
}

void llc_set_station_handler(void (*handler)(struct sk_buff *skb))
{
	llc_station_handler = handler;
}

/**
 *	llc_pdu_type - returns which LLC component must handle for PDU
	int dest;
	int (*rcv)(struct sk_buff *, struct net_device *,
		   struct packet_type *, struct net_device *);

	if (!net_eq(dev_net(dev), &init_net))
		goto drop;

	 */
	rcv = rcu_dereference(sap->rcv_func);
	dest = llc_pdu_type(skb);
	if (unlikely(!dest || !llc_type_handlers[dest - 1])) {
		if (rcv)
			rcv(skb, dev, pt, orig_dev);
		else
			kfree_skb(skb);
			if (cskb)
				rcv(cskb, dev, pt, orig_dev);
		}
		llc_type_handlers[dest - 1](sap, skb);
	}
	llc_sap_put(sap);
out:
	return 0;
	kfree_skb(skb);
	goto out;
handle_station:
	if (!llc_station_handler)
		goto drop;
	llc_station_handler(skb);
	goto out;
}

EXPORT_SYMBOL(llc_add_pack);