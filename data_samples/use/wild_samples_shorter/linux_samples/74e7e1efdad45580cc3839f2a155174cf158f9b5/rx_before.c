	return false;
}

void xenvif_rx_queue_tail(struct xenvif_queue *queue, struct sk_buff *skb)
{
	unsigned long flags;

	spin_lock_irqsave(&queue->rx_queue.lock, flags);

	if (queue->rx_queue_len >= queue->rx_queue_max) {
		struct net_device *dev = queue->vif->dev;

		netif_tx_stop_queue(netdev_get_tx_queue(dev, queue->id));
		kfree_skb(skb);
		queue->vif->dev->stats.rx_dropped++;
	} else {
		if (skb_queue_empty(&queue->rx_queue))
			xenvif_update_needed_slots(queue, skb);

	}

	spin_unlock_irqrestore(&queue->rx_queue.lock, flags);
}

static struct sk_buff *xenvif_rx_dequeue(struct xenvif_queue *queue)
{