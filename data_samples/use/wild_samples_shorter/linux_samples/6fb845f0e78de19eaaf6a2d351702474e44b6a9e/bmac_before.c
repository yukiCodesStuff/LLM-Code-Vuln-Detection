
		if (bp->tx_bufs[bp->tx_empty]) {
			++dev->stats.tx_packets;
			dev_kfree_skb_irq(bp->tx_bufs[bp->tx_empty]);
		}
		bp->tx_bufs[bp->tx_empty] = NULL;
		bp->tx_fullup = 0;
		netif_wake_queue(dev);