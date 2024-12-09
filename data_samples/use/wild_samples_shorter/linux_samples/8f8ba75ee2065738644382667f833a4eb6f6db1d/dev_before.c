	return pt_prev->func(skb, skb->dev, pt_prev, orig_dev);
}

/*
 *	Support routine. Sends outgoing frames to any network
 *	taps currently in use.
 */
		 * they originated from - MvS (miquels@drinkel.ow.org)
		 */
		if ((ptype->dev == dev || !ptype->dev) &&
		    (ptype->af_packet_priv == NULL ||
		     (struct sock *)ptype->af_packet_priv != skb->sk)) {
			if (pt_prev) {
				deliver_skb(skb2, pt_prev, skb->dev);
				pt_prev = ptype;
				continue;

/**
 * netdev_wait_allrefs - wait until all references are gone.
 *
 * This is called when unregistering network devices.
 *
 * Any protocol or device that holds a reference should register