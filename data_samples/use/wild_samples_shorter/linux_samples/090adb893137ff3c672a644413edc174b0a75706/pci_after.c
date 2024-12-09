				 "can't alloc skb for rx\n");
			goto done;
		}
		kmemleak_not_leak(new_skb);

		pci_unmap_single(rtlpci->pdev,
				 *((dma_addr_t *) skb->cb),
				 rtlpci->rxbuffersize,