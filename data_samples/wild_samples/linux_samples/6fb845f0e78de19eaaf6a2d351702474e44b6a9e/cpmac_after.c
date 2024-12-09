	if (likely(desc->skb)) {
		spin_lock(&priv->lock);
		dev->stats.tx_packets++;
		dev->stats.tx_bytes += desc->skb->len;
		spin_unlock(&priv->lock);
		dma_unmap_single(&dev->dev, desc->data_mapping, desc->skb->len,
				 DMA_TO_DEVICE);

		if (unlikely(netif_msg_tx_done(priv)))
			netdev_dbg(dev, "sent 0x%p, len=%d\n",
				   desc->skb, desc->skb->len);

		dev_consume_skb_irq(desc->skb);
		desc->skb = NULL;
		if (__netif_subqueue_stopped(dev, queue))
			netif_wake_subqueue(dev, queue);
	} else {
		if (netif_msg_tx_err(priv) && net_ratelimit())
			netdev_warn(dev, "end_xmit: spurious interrupt\n");
		if (__netif_subqueue_stopped(dev, queue))
			netif_wake_subqueue(dev, queue);
	}