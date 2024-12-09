	while (elem != bp->tx_new) {
		struct sk_buff *skb;
		struct be_txd *this = &txbase[elem];

		DTX(("this(%p) [flags(%08x)addr(%08x)]",
		     this, this->tx_flags, this->tx_addr));

		if (this->tx_flags & TXD_OWN)
			break;
		skb = bp->tx_skbs[elem];
		dev->stats.tx_packets++;
		dev->stats.tx_bytes += skb->len;
		dma_unmap_single(&bp->bigmac_op->dev,
				 this->tx_addr, skb->len,
				 DMA_TO_DEVICE);

		DTX(("skb(%p) ", skb));
		bp->tx_skbs[elem] = NULL;
		dev_kfree_skb_irq(skb);

		elem = NEXT_TX(elem);
	}
	DTX((" DONE, tx_old=%d\n", elem));
	bp->tx_old = elem;

	if (netif_queue_stopped(dev) &&
	    TX_BUFFS_AVAIL(bp) > 0)
		netif_wake_queue(bp->dev);

	spin_unlock(&bp->lock);
}

/* BigMAC receive complete service routines. */
static void bigmac_rx(struct bigmac *bp)
{
	struct be_rxd *rxbase = &bp->bmac_block->be_rxd[0];
	struct be_rxd *this;
	int elem = bp->rx_new, drops = 0;
	u32 flags;

	this = &rxbase[elem];
	while (!((flags = this->rx_flags) & RXD_OWN)) {
		struct sk_buff *skb;
		int len = (flags & RXD_LENGTH); /* FCS not included */

		/* Check for errors. */
		if (len < ETH_ZLEN) {
			bp->dev->stats.rx_errors++;
			bp->dev->stats.rx_length_errors++;

	drop_it:
			/* Return it to the BigMAC. */
			bp->dev->stats.rx_dropped++;
			this->rx_flags =
				(RXD_OWN | ((RX_BUF_ALLOC_SIZE - 34) & RXD_LENGTH));
			goto next;
		}