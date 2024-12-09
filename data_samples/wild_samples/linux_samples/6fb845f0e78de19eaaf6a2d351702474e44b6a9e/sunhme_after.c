		for (frag = 0; frag <= skb_shinfo(skb)->nr_frags; frag++) {
			dma_addr = hme_read_desc32(hp, &this->tx_addr);
			dma_len = hme_read_desc32(hp, &this->tx_flags);

			dma_len &= TXFLAG_SIZE;
			if (!frag)
				dma_unmap_single(hp->dma_dev, dma_addr, dma_len, DMA_TO_DEVICE);
			else
				dma_unmap_page(hp->dma_dev, dma_addr, dma_len, DMA_TO_DEVICE);

			elem = NEXT_TX(elem);
			this = &txbase[elem];
		}

		dev_consume_skb_irq(skb);
		dev->stats.tx_packets++;
	}
	hp->tx_old = elem;
	TXD((">"));

	if (netif_queue_stopped(dev) &&
	    TX_BUFFS_AVAIL(hp) > (MAX_SKB_FRAGS + 1))
		netif_wake_queue(dev);
}

#ifdef RXDEBUG
#define RXD(x) printk x
#else
#define RXD(x)
#endif

/* Originally I used to handle the allocation failure by just giving back just
 * that one ring buffer to the happy meal.  Problem is that usually when that
 * condition is triggered, the happy meal expects you to do something reasonable
 * with all of the packets it has DMA'd in.  So now I just drop the entire
 * ring when we cannot get a new skb and give them all back to the happy meal,
 * maybe things will be "happier" now.
 *
 * hp->happy_lock must be held
 */
static void happy_meal_rx(struct happy_meal *hp, struct net_device *dev)
{
	struct happy_meal_rxd *rxbase = &hp->happy_block->happy_meal_rxd[0];
	struct happy_meal_rxd *this;
	int elem = hp->rx_new, drops = 0;
	u32 flags;

	RXD(("RX<"));
	this = &rxbase[elem];
	while (!((flags = hme_read_desc32(hp, &this->rx_flags)) & RXFLAG_OWN)) {
		struct sk_buff *skb;
		int len = flags >> 16;
		u16 csum = flags & RXFLAG_CSUM;
		u32 dma_addr = hme_read_desc32(hp, &this->rx_addr);

		RXD(("[%d ", elem));

		/* Check for errors. */
		if ((len < ETH_ZLEN) || (flags & RXFLAG_OVERFLOW)) {
			RXD(("ERR(%08x)]", flags));
			dev->stats.rx_errors++;
			if (len < ETH_ZLEN)
				dev->stats.rx_length_errors++;
			if (len & (RXFLAG_OVERFLOW >> 16)) {
				dev->stats.rx_over_errors++;
				dev->stats.rx_fifo_errors++;
			}