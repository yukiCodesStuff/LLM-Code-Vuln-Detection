			}
		}
		rcu_read_unlock();
next:
		__skb_queue_tail(list, skb);
		if (!(rx->flags & XEN_NETRXF_more_data))
			break;

		if (cons + slots == rp) {