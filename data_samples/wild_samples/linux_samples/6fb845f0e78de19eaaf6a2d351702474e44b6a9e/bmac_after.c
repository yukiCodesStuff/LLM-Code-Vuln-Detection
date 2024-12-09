		if (bp->tx_bufs[bp->tx_empty]) {
			++dev->stats.tx_packets;
			dev_consume_skb_irq(bp->tx_bufs[bp->tx_empty]);
		}