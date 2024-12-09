{
	struct sk_buff *skb;
	struct chnl_net *priv;
	int pktlen;
	const u8 *ip_version;
	u8 buf;

	priv = container_of(layr, struct chnl_net, chnl);
	if (!priv)
		return -EINVAL;

	skb = (struct sk_buff *) cfpkt_tonative(pkt);

	/* Get length of CAIF packet. */
	pktlen = skb->len;

	/* Pass some minimum information and
	 * send the packet to the net stack.
	 */
	skb->dev = priv->netdev;

	/* check the version of IP */
	ip_version = skb_header_pointer(skb, 0, 1, &buf);

	switch (*ip_version >> 4) {
	case 4:
		skb->protocol = htons(ETH_P_IP);
		break;
	case 6:
		skb->protocol = htons(ETH_P_IPV6);
		break;
	default:
		kfree_skb(skb);
		priv->netdev->stats.rx_errors++;
		return -EINVAL;
	}