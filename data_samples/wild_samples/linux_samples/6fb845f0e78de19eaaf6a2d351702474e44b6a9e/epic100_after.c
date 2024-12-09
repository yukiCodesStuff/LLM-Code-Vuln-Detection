		if (likely(txstatus & 0x0001)) {
			dev->stats.collisions += (txstatus >> 8) & 15;
			dev->stats.tx_packets++;
			dev->stats.tx_bytes += ep->tx_skbuff[entry]->len;
		} else
			epic_tx_error(dev, ep, txstatus);

		/* Free the original skb. */
		skb = ep->tx_skbuff[entry];
		pci_unmap_single(ep->pci_dev, ep->tx_ring[entry].bufaddr,
				 skb->len, PCI_DMA_TODEVICE);
		dev_consume_skb_irq(skb);
		ep->tx_skbuff[entry] = NULL;
	}

#ifndef final_version
	if (cur_tx - dirty_tx > TX_RING_SIZE) {
		netdev_warn(dev, "Out-of-sync dirty pointer, %d vs. %d, full=%d.\n",
			    dirty_tx, cur_tx, ep->tx_full);
		dirty_tx += TX_RING_SIZE;
	}
#endif
	ep->dirty_tx = dirty_tx;
	if (ep->tx_full && cur_tx - dirty_tx < TX_QUEUE_LEN - 4) {
		/* The ring is no longer full, allow new TX entries. */
		ep->tx_full = 0;
		netif_wake_queue(dev);
	}
}

/* The interrupt handler does all of the Rx thread work and cleans up
   after the Tx thread. */
static irqreturn_t epic_interrupt(int irq, void *dev_instance)
{
	struct net_device *dev = dev_instance;
	struct epic_private *ep = netdev_priv(dev);
	void __iomem *ioaddr = ep->ioaddr;
	unsigned int handled = 0;
	int status;

	status = er32(INTSTAT);
	/* Acknowledge all of the current interrupt sources ASAP. */
	ew32(INTSTAT, status & EpicNormalEvent);

	if (debug > 4) {
		netdev_dbg(dev, "Interrupt, status=%#8.8x new intstat=%#8.8x.\n",
			   status, er32(INTSTAT));
	}

	if ((status & IntrSummary) == 0)
		goto out;

	handled = 1;

	if (status & EpicNapiEvent) {
		spin_lock(&ep->napi_lock);
		if (napi_schedule_prep(&ep->napi)) {
			epic_napi_irq_off(dev, ep);
			__napi_schedule(&ep->napi);
		}