	for (cons = bp->tx_cons; cons != cur; cons = NEXT_TX(cons)) {
		struct ring_info *rp = &bp->tx_buffers[cons];
		struct sk_buff *skb = rp->skb;

		BUG_ON(skb == NULL);

		dma_unmap_single(bp->sdev->dma_dev,
				 rp->mapping,
				 skb->len,
				 DMA_TO_DEVICE);
		rp->skb = NULL;

		bytes_compl += skb->len;
		pkts_compl++;

		dev_kfree_skb_irq(skb);
	}

	netdev_completed_queue(bp->dev, pkts_compl, bytes_compl);
	bp->tx_cons = cons;
	if (netif_queue_stopped(bp->dev) &&
	    TX_BUFFS_AVAIL(bp) > B44_TX_WAKEUP_THRESH)
		netif_wake_queue(bp->dev);

	bw32(bp, B44_GPTIMER, 0);
}

/* Works like this.  This chip writes a 'struct rx_header" 30 bytes
 * before the DMA address you give it.  So we allocate 30 more bytes
 * for the RX buffer, DMA map all of it, skb_reserve the 30 bytes, then
 * point the chip at 30 bytes past where the rx_header will go.
 */
static int b44_alloc_rx_skb(struct b44 *bp, int src_idx, u32 dest_idx_unmasked)
{
	struct dma_desc *dp;
	struct ring_info *src_map, *map;
	struct rx_header *rh;
	struct sk_buff *skb;
	dma_addr_t mapping;
	int dest_idx;
	u32 ctrl;

	src_map = NULL;
	if (src_idx >= 0)
		src_map = &bp->rx_buffers[src_idx];
	dest_idx = dest_idx_unmasked & (B44_RX_RING_SIZE - 1);
	map = &bp->rx_buffers[dest_idx];
	skb = netdev_alloc_skb(bp->dev, RX_PKT_BUF_SZ);
	if (skb == NULL)
		return -ENOMEM;

	mapping = dma_map_single(bp->sdev->dma_dev, skb->data,
				 RX_PKT_BUF_SZ,
				 DMA_FROM_DEVICE);

	/* Hardware bug work-around, the chip is unable to do PCI DMA
	   to/from anything above 1GB :-( */
	if (dma_mapping_error(bp->sdev->dma_dev, mapping) ||
		mapping + RX_PKT_BUF_SZ > DMA_BIT_MASK(30)) {
		/* Sigh... */
		if (!dma_mapping_error(bp->sdev->dma_dev, mapping))
			dma_unmap_single(bp->sdev->dma_dev, mapping,
					     RX_PKT_BUF_SZ, DMA_FROM_DEVICE);
		dev_kfree_skb_any(skb);
		skb = alloc_skb(RX_PKT_BUF_SZ, GFP_ATOMIC | GFP_DMA);
		if (skb == NULL)
			return -ENOMEM;
		mapping = dma_map_single(bp->sdev->dma_dev, skb->data,
					 RX_PKT_BUF_SZ,
					 DMA_FROM_DEVICE);
		if (dma_mapping_error(bp->sdev->dma_dev, mapping) ||
		    mapping + RX_PKT_BUF_SZ > DMA_BIT_MASK(30)) {
			if (!dma_mapping_error(bp->sdev->dma_dev, mapping))
				dma_unmap_single(bp->sdev->dma_dev, mapping, RX_PKT_BUF_SZ,DMA_FROM_DEVICE);
			dev_kfree_skb_any(skb);
			return -ENOMEM;
		}
		if (dma_mapping_error(bp->sdev->dma_dev, mapping) || mapping + len > DMA_BIT_MASK(30)) {
			if (!dma_mapping_error(bp->sdev->dma_dev, mapping))
				dma_unmap_single(bp->sdev->dma_dev, mapping,
						     len, DMA_TO_DEVICE);
			dev_kfree_skb_any(bounce_skb);
			goto err_out;
		}

		skb_copy_from_linear_data(skb, skb_put(bounce_skb, len), len);
		dev_kfree_skb_any(skb);
		skb = bounce_skb;
	}

	entry = bp->tx_prod;
	bp->tx_buffers[entry].skb = skb;
	bp->tx_buffers[entry].mapping = mapping;

	ctrl  = (len & DESC_CTRL_LEN);
	ctrl |= DESC_CTRL_IOC | DESC_CTRL_SOF | DESC_CTRL_EOF;
	if (entry == (B44_TX_RING_SIZE - 1))
		ctrl |= DESC_CTRL_EOT;

	bp->tx_ring[entry].ctrl = cpu_to_le32(ctrl);
	bp->tx_ring[entry].addr = cpu_to_le32((u32) mapping+bp->dma_offset);

	if (bp->flags & B44_FLAG_TX_RING_HACK)
		b44_sync_dma_desc_for_device(bp->sdev, bp->tx_ring_dma,
			                    entry * sizeof(bp->tx_ring[0]),
			                    DMA_TO_DEVICE);

	entry = NEXT_TX(entry);

	bp->tx_prod = entry;

	wmb();

	bw32(bp, B44_DMATX_PTR, entry * sizeof(struct dma_desc));
	if (bp->flags & B44_FLAG_BUGGY_TXPTR)
		bw32(bp, B44_DMATX_PTR, entry * sizeof(struct dma_desc));
	if (bp->flags & B44_FLAG_REORDER_BUG)
		br32(bp, B44_DMATX_PTR);

	netdev_sent_queue(dev, skb->len);

	if (TX_BUFFS_AVAIL(bp) < 1)
		netif_stop_queue(dev);

out_unlock:
	spin_unlock_irqrestore(&bp->lock, flags);

	return rc;

err_out:
	rc = NETDEV_TX_BUSY;
	goto out_unlock;
}

static int b44_change_mtu(struct net_device *dev, int new_mtu)
{
	struct b44 *bp = netdev_priv(dev);

	if (!netif_running(dev)) {
		/* We'll just catch it later when the
		 * device is up'd.
		 */
		dev->mtu = new_mtu;
		return 0;
	}

	spin_lock_irq(&bp->lock);
	b44_halt(bp);
	dev->mtu = new_mtu;
	b44_init_rings(bp);
	b44_init_hw(bp, B44_FULL_RESET);
	spin_unlock_irq(&bp->lock);

	b44_enable_ints(bp);

	return 0;
}

/* Free up pending packets in all rx/tx rings.
 *
 * The chip has been shut down and the driver detached from
 * the networking, so no interrupts or new tx packets will
 * end up in the driver.  bp->lock is not held and we are not
 * in an interrupt context and thus may sleep.
 */
static void b44_free_rings(struct b44 *bp)
{
	struct ring_info *rp;
	int i;

	for (i = 0; i < B44_RX_RING_SIZE; i++) {
		rp = &bp->rx_buffers[i];

		if (rp->skb == NULL)
			continue;
		dma_unmap_single(bp->sdev->dma_dev, rp->mapping, RX_PKT_BUF_SZ,
				 DMA_FROM_DEVICE);
		dev_kfree_skb_any(rp->skb);
		rp->skb = NULL;
	}

	/* XXX needs changes once NETIF_F_SG is set... */
	for (i = 0; i < B44_TX_RING_SIZE; i++) {
		rp = &bp->tx_buffers[i];

		if (rp->skb == NULL)
			continue;
		dma_unmap_single(bp->sdev->dma_dev, rp->mapping, rp->skb->len,
				 DMA_TO_DEVICE);
		dev_kfree_skb_any(rp->skb);
		rp->skb = NULL;
	}
}

/* Initialize tx/rx rings for packet processing.
 *
 * The chip has been shut down and the driver detached from
 * the networking, so no interrupts or new tx packets will
 * end up in the driver.
 */
static void b44_init_rings(struct b44 *bp)
{
	int i;

	b44_free_rings(bp);

	memset(bp->rx_ring, 0, B44_RX_RING_BYTES);
	memset(bp->tx_ring, 0, B44_TX_RING_BYTES);

	if (bp->flags & B44_FLAG_RX_RING_HACK)
		dma_sync_single_for_device(bp->sdev->dma_dev, bp->rx_ring_dma,
					   DMA_TABLE_BYTES, DMA_BIDIRECTIONAL);

	if (bp->flags & B44_FLAG_TX_RING_HACK)
		dma_sync_single_for_device(bp->sdev->dma_dev, bp->tx_ring_dma,
					   DMA_TABLE_BYTES, DMA_TO_DEVICE);

	for (i = 0; i < bp->rx_pending; i++) {
		if (b44_alloc_rx_skb(bp, -1, i) < 0)
			break;
	}
}

/*
 * Must not be invoked with interrupt sources disabled and
 * the hardware shutdown down.
 */
static void b44_free_consistent(struct b44 *bp)
{
	kfree(bp->rx_buffers);
	bp->rx_buffers = NULL;
	kfree(bp->tx_buffers);
	bp->tx_buffers = NULL;
	if (bp->rx_ring) {
		if (bp->flags & B44_FLAG_RX_RING_HACK) {
			dma_unmap_single(bp->sdev->dma_dev, bp->rx_ring_dma,
					 DMA_TABLE_BYTES, DMA_BIDIRECTIONAL);
			kfree(bp->rx_ring);
		} else
			dma_free_coherent(bp->sdev->dma_dev, DMA_TABLE_BYTES,
					  bp->rx_ring, bp->rx_ring_dma);
		bp->rx_ring = NULL;
		bp->flags &= ~B44_FLAG_RX_RING_HACK;
	}