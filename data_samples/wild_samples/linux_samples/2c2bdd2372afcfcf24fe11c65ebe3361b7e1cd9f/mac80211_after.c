{
	struct sk_buff *skb = phy->rx_amsdu[q].head;
	struct mt76_rx_status *status = (struct mt76_rx_status *)skb->cb;
	struct mt76_dev *dev = phy->dev;

	phy->rx_amsdu[q].head = NULL;
	phy->rx_amsdu[q].tail = NULL;

	/*
	 * Validate if the amsdu has a proper first subframe.
	 * A single MSDU can be parsed as A-MSDU when the unauthenticated A-MSDU
	 * flag of the QoS header gets flipped. In such cases, the first
	 * subframe has a LLC/SNAP header in the location of the destination
	 * address.
	 */
	if (skb_shinfo(skb)->frag_list) {
		int offset = 0;

		if (!(status->flag & RX_FLAG_8023)) {
			offset = ieee80211_get_hdrlen_from_skb(skb);

			if ((status->flag &
			     (RX_FLAG_DECRYPTED | RX_FLAG_IV_STRIPPED)) ==
			    RX_FLAG_DECRYPTED)
				offset += 8;
		}

		if (ether_addr_equal(skb->data + offset, rfc1042_header)) {
			dev_kfree_skb(skb);
			return;
		}
	}
	__skb_queue_tail(&dev->rx_skb[q], skb);
}