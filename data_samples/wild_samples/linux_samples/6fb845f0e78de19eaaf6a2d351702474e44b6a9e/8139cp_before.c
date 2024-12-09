			} else {
				cp->dev->stats.collisions +=
					((status >> TxColCntShift) & TxColCntMask);
				cp->dev->stats.tx_packets++;
				cp->dev->stats.tx_bytes += skb->len;
				netif_dbg(cp, tx_done, cp->dev,
					  "tx done, slot %d\n", tx_tail);
			}
			bytes_compl += skb->len;
			pkts_compl++;
			dev_kfree_skb_irq(skb);
		}

		cp->tx_skb[tx_tail] = NULL;

		tx_tail = NEXT_TX(tx_tail);
	}

	cp->tx_tail = tx_tail;

	netdev_completed_queue(cp->dev, pkts_compl, bytes_compl);
	if (TX_BUFFS_AVAIL(cp) > (MAX_SKB_FRAGS + 1))
		netif_wake_queue(cp->dev);
}

static inline u32 cp_tx_vlan_tag(struct sk_buff *skb)
{
	return skb_vlan_tag_present(skb) ?
		TxVlanTag | swab16(skb_vlan_tag_get(skb)) : 0x00;
}

static void unwind_tx_frag_mapping(struct cp_private *cp, struct sk_buff *skb,
				   int first, int entry_last)
{
	int frag, index;
	struct cp_desc *txd;
	skb_frag_t *this_frag;
	for (frag = 0; frag+first < entry_last; frag++) {
		index = first+frag;
		cp->tx_skb[index] = NULL;
		txd = &cp->tx_ring[index];
		this_frag = &skb_shinfo(skb)->frags[frag];
		dma_unmap_single(&cp->pdev->dev, le64_to_cpu(txd->addr),
				 skb_frag_size(this_frag), PCI_DMA_TODEVICE);
	}
}

static netdev_tx_t cp_start_xmit (struct sk_buff *skb,
					struct net_device *dev)
{
	struct cp_private *cp = netdev_priv(dev);
	unsigned entry;
	u32 eor, opts1;
	unsigned long intr_flags;
	__le32 opts2;
	int mss = 0;

	spin_lock_irqsave(&cp->lock, intr_flags);

	/* This is a hard error, log it. */
	if (TX_BUFFS_AVAIL(cp) <= (skb_shinfo(skb)->nr_frags + 1)) {
		netif_stop_queue(dev);
		spin_unlock_irqrestore(&cp->lock, intr_flags);
		netdev_err(dev, "BUG! Tx Ring full when queue awake!\n");
		return NETDEV_TX_BUSY;
	}

	entry = cp->tx_head;
	eor = (entry == (CP_TX_RING_SIZE - 1)) ? RingEnd : 0;
	mss = skb_shinfo(skb)->gso_size;

	if (mss > MSSMask) {
		netdev_WARN_ONCE(dev, "Net bug: GSO size %d too large for 8139CP\n",
				 mss);
		goto out_dma_error;
	}

	opts2 = cpu_to_le32(cp_tx_vlan_tag(skb));
	opts1 = DescOwn;
	if (mss)
		opts1 |= LargeSend | (mss << MSSShift);
	else if (skb->ip_summed == CHECKSUM_PARTIAL) {
		const struct iphdr *ip = ip_hdr(skb);
		if (ip->protocol == IPPROTO_TCP)
			opts1 |= IPCS | TCPCS;
		else if (ip->protocol == IPPROTO_UDP)
			opts1 |= IPCS | UDPCS;
		else {
			WARN_ONCE(1,
				  "Net bug: asked to checksum invalid Legacy IP packet\n");
			goto out_dma_error;
		}
	}

	if (skb_shinfo(skb)->nr_frags == 0) {
		struct cp_desc *txd = &cp->tx_ring[entry];
		u32 len;
		dma_addr_t mapping;

		len = skb->len;
		mapping = dma_map_single(&cp->pdev->dev, skb->data, len, PCI_DMA_TODEVICE);
		if (dma_mapping_error(&cp->pdev->dev, mapping))
			goto out_dma_error;

		txd->opts2 = opts2;
		txd->addr = cpu_to_le64(mapping);
		wmb();

		opts1 |= eor | len | FirstFrag | LastFrag;

		txd->opts1 = cpu_to_le32(opts1);
		wmb();

		cp->tx_skb[entry] = skb;
		cp->tx_opts[entry] = opts1;
		netif_dbg(cp, tx_queued, cp->dev, "tx queued, slot %d, skblen %d\n",
			  entry, skb->len);
	} else {
		struct cp_desc *txd;
		u32 first_len, first_eor, ctrl;
		dma_addr_t first_mapping;
		int frag, first_entry = entry;

		/* We must give this initial chunk to the device last.
		 * Otherwise we could race with the device.
		 */
		first_eor = eor;
		first_len = skb_headlen(skb);
		first_mapping = dma_map_single(&cp->pdev->dev, skb->data,
					       first_len, PCI_DMA_TODEVICE);
		if (dma_mapping_error(&cp->pdev->dev, first_mapping))
			goto out_dma_error;

		cp->tx_skb[entry] = skb;

		for (frag = 0; frag < skb_shinfo(skb)->nr_frags; frag++) {
			const skb_frag_t *this_frag = &skb_shinfo(skb)->frags[frag];
			u32 len;
			dma_addr_t mapping;

			entry = NEXT_TX(entry);

			len = skb_frag_size(this_frag);
			mapping = dma_map_single(&cp->pdev->dev,
						 skb_frag_address(this_frag),
						 len, PCI_DMA_TODEVICE);
			if (dma_mapping_error(&cp->pdev->dev, mapping)) {
				unwind_tx_frag_mapping(cp, skb, first_entry, entry);
				goto out_dma_error;
			}

			eor = (entry == (CP_TX_RING_SIZE - 1)) ? RingEnd : 0;

			ctrl = opts1 | eor | len;

			if (frag == skb_shinfo(skb)->nr_frags - 1)
				ctrl |= LastFrag;

			txd = &cp->tx_ring[entry];
			txd->opts2 = opts2;
			txd->addr = cpu_to_le64(mapping);
			wmb();

			txd->opts1 = cpu_to_le32(ctrl);
			wmb();

			cp->tx_opts[entry] = ctrl;
			cp->tx_skb[entry] = skb;
		}