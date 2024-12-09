	return pt_prev->func(skb, skb->dev, pt_prev, orig_dev);
}

static inline bool skb_loop_sk(struct packet_type *ptype, struct sk_buff *skb)
{
	if (ptype->af_packet_priv == NULL)
		return false;

	if (ptype->id_match)
		return ptype->id_match(ptype, skb->sk);
	else if ((struct sock *)ptype->af_packet_priv == skb->sk)
		return true;

	return false;
}

/*
 *	Support routine. Sends outgoing frames to any network
 *	taps currently in use.
 */
		 * they originated from - MvS (miquels@drinkel.ow.org)
		 */
		if ((ptype->dev == dev || !ptype->dev) &&
		    (!skb_loop_sk(ptype, skb))) {
			if (pt_prev) {
				deliver_skb(skb2, pt_prev, skb->dev);
				pt_prev = ptype;
				continue;

/**
 * netdev_wait_allrefs - wait until all references are gone.
 * @dev: target net_device
 *
 * This is called when unregistering network devices.
 *
 * Any protocol or device that holds a reference should register