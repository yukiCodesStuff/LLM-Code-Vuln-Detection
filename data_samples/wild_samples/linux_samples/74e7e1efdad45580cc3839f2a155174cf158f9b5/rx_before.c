	do {
		prod = queue->rx.sring->req_prod;
		cons = queue->rx.req_cons;

		if (prod - cons >= needed)
			return true;

		queue->rx.sring->req_event = prod + 1;

		/* Make sure event is visible before we check prod
		 * again.
		 */
		mb();
	} while (queue->rx.sring->req_prod != prod);

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

		__skb_queue_tail(&queue->rx_queue, skb);

		queue->rx_queue_len += skb->len;
	}

	spin_unlock_irqrestore(&queue->rx_queue.lock, flags);
}
	} else {
		if (skb_queue_empty(&queue->rx_queue))
			xenvif_update_needed_slots(queue, skb);

		__skb_queue_tail(&queue->rx_queue, skb);

		queue->rx_queue_len += skb->len;
	}

	spin_unlock_irqrestore(&queue->rx_queue.lock, flags);
}

static struct sk_buff *xenvif_rx_dequeue(struct xenvif_queue *queue)
{
	struct sk_buff *skb;

	spin_lock_irq(&queue->rx_queue.lock);

	skb = __skb_dequeue(&queue->rx_queue);
	if (skb) {
		xenvif_update_needed_slots(queue, skb_peek(&queue->rx_queue));

		queue->rx_queue_len -= skb->len;
		if (queue->rx_queue_len < queue->rx_queue_max) {
			struct netdev_queue *txq;

			txq = netdev_get_tx_queue(queue->vif->dev, queue->id);
			netif_tx_wake_queue(txq);
		}
	}