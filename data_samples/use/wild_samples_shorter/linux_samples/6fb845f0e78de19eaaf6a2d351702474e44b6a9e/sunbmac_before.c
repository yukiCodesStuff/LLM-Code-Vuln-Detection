
		DTX(("skb(%p) ", skb));
		bp->tx_skbs[elem] = NULL;
		dev_kfree_skb_irq(skb);

		elem = NEXT_TX(elem);
	}
	DTX((" DONE, tx_old=%d\n", elem));