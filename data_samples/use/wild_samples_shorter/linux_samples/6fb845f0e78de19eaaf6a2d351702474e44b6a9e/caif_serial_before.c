		if (skb->len == 0) {
			struct sk_buff *tmp = skb_dequeue(&ser->head);
			WARN_ON(tmp != skb);
			if (in_interrupt())
				dev_kfree_skb_irq(skb);
			else
				kfree_skb(skb);
		}
	}
	/* Send flow off if queue is empty */
	if (ser->head.qlen <= SEND_QUEUE_LOW &&