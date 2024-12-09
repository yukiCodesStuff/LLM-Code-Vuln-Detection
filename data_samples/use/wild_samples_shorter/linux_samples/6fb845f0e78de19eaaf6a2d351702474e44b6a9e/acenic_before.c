		if (skb) {
			dev->stats.tx_packets++;
			dev->stats.tx_bytes += skb->len;
			dev_kfree_skb_irq(skb);
			info->skb = NULL;
		}

		idx = (idx + 1) % ACE_TX_RING_ENTRIES(ap);