		if (skb->len == 0) {
			struct sk_buff *tmp = skb_dequeue(&ser->head);
			WARN_ON(tmp != skb);
			dev_consume_skb_any(skb);
		}
	}
	/* Send flow off if queue is empty */
	if (ser->head.qlen <= SEND_QUEUE_LOW &&