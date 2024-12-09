		bytes_compl += skb->len;
		pkts_compl++;

		dev_kfree_skb_irq(skb);
	}

	netdev_completed_queue(bp->dev, pkts_compl, bytes_compl);
	bp->tx_cons = cons;
		}

		skb_copy_from_linear_data(skb, skb_put(bounce_skb, len), len);
		dev_kfree_skb_any(skb);
		skb = bounce_skb;
	}

	entry = bp->tx_prod;