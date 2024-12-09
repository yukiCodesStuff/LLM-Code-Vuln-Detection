			}
			bytes_compl += skb->len;
			pkts_compl++;
			dev_kfree_skb_irq(skb);
		}

		cp->tx_skb[tx_tail] = NULL;
