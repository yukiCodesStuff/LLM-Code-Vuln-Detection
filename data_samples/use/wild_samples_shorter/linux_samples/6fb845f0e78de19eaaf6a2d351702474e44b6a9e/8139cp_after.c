			}
			bytes_compl += skb->len;
			pkts_compl++;
			dev_consume_skb_irq(skb);
		}

		cp->tx_skb[tx_tail] = NULL;
