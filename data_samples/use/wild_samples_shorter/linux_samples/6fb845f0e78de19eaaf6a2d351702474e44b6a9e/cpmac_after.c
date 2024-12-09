			netdev_dbg(dev, "sent 0x%p, len=%d\n",
				   desc->skb, desc->skb->len);

		dev_consume_skb_irq(desc->skb);
		desc->skb = NULL;
		if (__netif_subqueue_stopped(dev, queue))
			netif_wake_subqueue(dev, queue);
	} else {