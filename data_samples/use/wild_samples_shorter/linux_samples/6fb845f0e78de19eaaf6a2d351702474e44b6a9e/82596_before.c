						dev->stats.tx_aborted_errors++;
				}

				dev_kfree_skb_irq(skb);

				tx_cmd->cmd.command = 0; /* Mark free */
				break;
			    }