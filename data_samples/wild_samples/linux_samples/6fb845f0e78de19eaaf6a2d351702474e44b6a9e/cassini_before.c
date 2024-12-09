			if (cp->tx_tiny_use[ring][entry].used) {
				cp->tx_tiny_use[ring][entry].used = 0;
				entry = TX_DESC_NEXT(ring, entry);
			}
		}

		spin_lock(&cp->stat_lock[ring]);
		cp->net_stats[ring].tx_packets++;
		cp->net_stats[ring].tx_bytes += skb->len;
		spin_unlock(&cp->stat_lock[ring]);
		dev_kfree_skb_irq(skb);
	}
	cp->tx_old[ring] = entry;

	/* this is wrong for multiple tx rings. the net device needs
	 * multiple queues for this to do the right thing.  we wait
	 * for 2*packets to be available when using tiny buffers
	 */
	if (netif_queue_stopped(dev) &&
	    (TX_BUFFS_AVAIL(cp, ring) > CAS_TABORT(cp)*(MAX_SKB_FRAGS + 1)))
		netif_wake_queue(dev);
	spin_unlock(&cp->tx_lock[ring]);
}

static void cas_tx(struct net_device *dev, struct cas *cp,
		   u32 status)
{
        int limit, ring;
#ifdef USE_TX_COMPWB
	u64 compwb = le64_to_cpu(cp->init_block->tx_compwb);
#endif
	netif_printk(cp, intr, KERN_DEBUG, cp->dev,
		     "tx interrupt, status: 0x%x, %llx\n",
		     status, (unsigned long long)compwb);
	/* process all the rings */
	for (ring = 0; ring < N_TX_RINGS; ring++) {
#ifdef USE_TX_COMPWB
		/* use the completion writeback registers */
		limit = (CAS_VAL(TX_COMPWB_MSB, compwb) << 8) |
			CAS_VAL(TX_COMPWB_LSB, compwb);
		compwb = TX_COMPWB_NEXT(compwb);
#else
		limit = readl(cp->regs + REG_TX_COMPN(ring));
#endif
		if (cp->tx_old[ring] != limit)
			cas_tx_ringN(cp, ring, limit);
	}
}


static int cas_rx_process_pkt(struct cas *cp, struct cas_rx_comp *rxc,
			      int entry, const u64 *words,
			      struct sk_buff **skbref)
{
	int dlen, hlen, len, i, alloclen;
	int off, swivel = RX_SWIVEL_OFF_VAL;
	struct cas_page *page;
	struct sk_buff *skb;
	void *addr, *crcaddr;
	__sum16 csum;
	char *p;

	hlen = CAS_VAL(RX_COMP2_HDR_SIZE, words[1]);
	dlen = CAS_VAL(RX_COMP1_DATA_SIZE, words[0]);
	len  = hlen + dlen;

	if (RX_COPY_ALWAYS || (words[2] & RX_COMP3_SMALL_PKT))
		alloclen = len;
	else
		alloclen = max(hlen, RX_COPY_MIN);

	skb = netdev_alloc_skb(cp->dev, alloclen + swivel + cp->crc_size);
	if (skb == NULL)
		return -1;

	*skbref = skb;
	skb_reserve(skb, swivel);

	p = skb->data;
	addr = crcaddr = NULL;
	if (hlen) { /* always copy header pages */
		i = CAS_VAL(RX_COMP2_HDR_INDEX, words[1]);
		page = cp->rx_pages[CAS_VAL(RX_INDEX_RING, i)][CAS_VAL(RX_INDEX_NUM, i)];
		off = CAS_VAL(RX_COMP2_HDR_OFF, words[1]) * 0x100 +
			swivel;

		i = hlen;
		if (!dlen) /* attach FCS */
			i += cp->crc_size;
		pci_dma_sync_single_for_cpu(cp->pdev, page->dma_addr + off, i,
				    PCI_DMA_FROMDEVICE);
		addr = cas_page_map(page->buffer);
		memcpy(p, addr + off, i);
		pci_dma_sync_single_for_device(cp->pdev, page->dma_addr + off, i,
				    PCI_DMA_FROMDEVICE);
		cas_page_unmap(addr);
		RX_USED_ADD(page, 0x100);
		p += hlen;
		swivel = 0;
	}


	if (alloclen < (hlen + dlen)) {
		skb_frag_t *frag = skb_shinfo(skb)->frags;

		/* normal or jumbo packets. we use frags */
		i = CAS_VAL(RX_COMP1_DATA_INDEX, words[0]);
		page = cp->rx_pages[CAS_VAL(RX_INDEX_RING, i)][CAS_VAL(RX_INDEX_NUM, i)];
		off = CAS_VAL(RX_COMP1_DATA_OFF, words[0]) + swivel;

		hlen = min(cp->page_size - off, dlen);
		if (hlen < 0) {
			netif_printk(cp, rx_err, KERN_DEBUG, cp->dev,
				     "rx page overflow: %d\n", hlen);
			dev_kfree_skb_irq(skb);
			return -1;
		}
		i = hlen;
		if (i == dlen)  /* attach FCS */
			i += cp->crc_size;
		pci_dma_sync_single_for_cpu(cp->pdev, page->dma_addr + off, i,
				    PCI_DMA_FROMDEVICE);

		/* make sure we always copy a header */
		swivel = 0;
		if (p == (char *) skb->data) { /* not split */
			addr = cas_page_map(page->buffer);
			memcpy(p, addr + off, RX_COPY_MIN);
			pci_dma_sync_single_for_device(cp->pdev, page->dma_addr + off, i,
					PCI_DMA_FROMDEVICE);
			cas_page_unmap(addr);
			off += RX_COPY_MIN;
			swivel = RX_COPY_MIN;
			RX_USED_ADD(page, cp->mtu_stride);
		} else {
			RX_USED_ADD(page, hlen);
		}
		skb_put(skb, alloclen);

		skb_shinfo(skb)->nr_frags++;
		skb->data_len += hlen - swivel;
		skb->truesize += hlen - swivel;
		skb->len      += hlen - swivel;

		__skb_frag_set_page(frag, page->buffer);
		__skb_frag_ref(frag);
		frag->page_offset = off;
		skb_frag_size_set(frag, hlen - swivel);

		/* any more data? */
		if ((words[0] & RX_COMP1_SPLIT_PKT) && ((dlen -= hlen) > 0)) {
			hlen = dlen;
			off = 0;

			i = CAS_VAL(RX_COMP2_NEXT_INDEX, words[1]);
			page = cp->rx_pages[CAS_VAL(RX_INDEX_RING, i)][CAS_VAL(RX_INDEX_NUM, i)];
			pci_dma_sync_single_for_cpu(cp->pdev, page->dma_addr,
					    hlen + cp->crc_size,
					    PCI_DMA_FROMDEVICE);
			pci_dma_sync_single_for_device(cp->pdev, page->dma_addr,
					    hlen + cp->crc_size,
					    PCI_DMA_FROMDEVICE);

			skb_shinfo(skb)->nr_frags++;
			skb->data_len += hlen;
			skb->len      += hlen;
			frag++;

			__skb_frag_set_page(frag, page->buffer);
			__skb_frag_ref(frag);
			frag->page_offset = 0;
			skb_frag_size_set(frag, hlen);
			RX_USED_ADD(page, hlen + cp->crc_size);
		}

		if (cp->crc_size) {
			addr = cas_page_map(page->buffer);
			crcaddr  = addr + off + hlen;
		}

	} else {
		/* copying packet */
		if (!dlen)
			goto end_copy_pkt;

		i = CAS_VAL(RX_COMP1_DATA_INDEX, words[0]);
		page = cp->rx_pages[CAS_VAL(RX_INDEX_RING, i)][CAS_VAL(RX_INDEX_NUM, i)];
		off = CAS_VAL(RX_COMP1_DATA_OFF, words[0]) + swivel;
		hlen = min(cp->page_size - off, dlen);
		if (hlen < 0) {
			netif_printk(cp, rx_err, KERN_DEBUG, cp->dev,
				     "rx page overflow: %d\n", hlen);
			dev_kfree_skb_irq(skb);
			return -1;
		}