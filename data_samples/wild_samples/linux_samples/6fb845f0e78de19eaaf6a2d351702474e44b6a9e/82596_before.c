				} else {
					dev->stats.tx_errors++;
					if ((ptr->status) & 0x0020)
						dev->stats.collisions++;
					if (!((ptr->status) & 0x0040))
						dev->stats.tx_heartbeat_errors++;
					if ((ptr->status) & 0x0400)
						dev->stats.tx_carrier_errors++;
					if ((ptr->status) & 0x0800)
						dev->stats.collisions++;
					if ((ptr->status) & 0x1000)
						dev->stats.tx_aborted_errors++;
				}

				dev_kfree_skb_irq(skb);

				tx_cmd->cmd.command = 0; /* Mark free */
				break;
			    }
			case CmdTDR:
			    {
				unsigned short status = ((struct tdr_cmd *)ptr)->status;

				if (status & 0x8000) {
					DEB(DEB_TDR,printk(KERN_INFO "%s: link ok.\n", dev->name));
				} else {
					if (status & 0x4000)
						printk(KERN_ERR "%s: Transceiver problem.\n", dev->name);
					if (status & 0x2000)
						printk(KERN_ERR "%s: Termination problem.\n", dev->name);
					if (status & 0x1000)
						printk(KERN_ERR "%s: Short circuit.\n", dev->name);

					DEB(DEB_TDR,printk(KERN_INFO "%s: Time %d.\n", dev->name, status & 0x07ff));
				}