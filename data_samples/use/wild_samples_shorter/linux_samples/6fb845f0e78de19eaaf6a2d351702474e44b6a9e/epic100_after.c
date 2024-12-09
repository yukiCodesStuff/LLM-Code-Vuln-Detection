		skb = ep->tx_skbuff[entry];
		pci_unmap_single(ep->pci_dev, ep->tx_ring[entry].bufaddr,
				 skb->len, PCI_DMA_TODEVICE);
		dev_consume_skb_irq(skb);
		ep->tx_skbuff[entry] = NULL;
	}

#ifndef final_version