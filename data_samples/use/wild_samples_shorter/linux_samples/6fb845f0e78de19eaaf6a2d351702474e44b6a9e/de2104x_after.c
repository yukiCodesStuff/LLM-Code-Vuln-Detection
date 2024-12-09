				netif_dbg(de, tx_done, de->dev,
					  "tx done, slot %d\n", tx_tail);
			}
			dev_consume_skb_irq(skb);
		}

next:
		de->tx_skb[tx_tail].skb = NULL;