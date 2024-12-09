		dma_unmap_single(dev->dev.parent, bd->skb_pa, skb->len,
				 DMA_TO_DEVICE);

		dev_consume_skb_irq(skb);
	}
	spin_unlock(&priv->lock);

	netif_wake_queue(dev);