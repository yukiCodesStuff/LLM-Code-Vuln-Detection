			} else {
				de->dev->stats.tx_packets++;
				de->dev->stats.tx_bytes += skb->len;
				netif_dbg(de, tx_done, de->dev,
					  "tx done, slot %d\n", tx_tail);
			}
			dev_kfree_skb_irq(skb);
		}

next:
		de->tx_skb[tx_tail].skb = NULL;

		tx_tail = NEXT_TX(tx_tail);
	}

	de->tx_tail = tx_tail;

	if (netif_queue_stopped(de->dev) && (TX_BUFFS_AVAIL(de) > (DE_TX_RING_SIZE / 4)))
		netif_wake_queue(de->dev);
}

static netdev_tx_t de_start_xmit (struct sk_buff *skb,
					struct net_device *dev)
{
	struct de_private *de = netdev_priv(dev);
	unsigned int entry, tx_free;
	u32 mapping, len, flags = FirstFrag | LastFrag;
	struct de_desc *txd;

	spin_lock_irq(&de->lock);

	tx_free = TX_BUFFS_AVAIL(de);
	if (tx_free == 0) {
		netif_stop_queue(dev);
		spin_unlock_irq(&de->lock);
		return NETDEV_TX_BUSY;
	}
	tx_free--;

	entry = de->tx_head;

	txd = &de->tx_ring[entry];

	len = skb->len;
	mapping = pci_map_single(de->pdev, skb->data, len, PCI_DMA_TODEVICE);
	if (entry == (DE_TX_RING_SIZE - 1))
		flags |= RingEnd;
	if (!tx_free || (tx_free == (DE_TX_RING_SIZE / 2)))
		flags |= TxSwInt;
	flags |= len;
	txd->opts2 = cpu_to_le32(flags);
	txd->addr1 = cpu_to_le32(mapping);

	de->tx_skb[entry].skb = skb;
	de->tx_skb[entry].mapping = mapping;
	wmb();

	txd->opts1 = cpu_to_le32(DescOwn);
	wmb();

	de->tx_head = NEXT_TX(entry);
	netif_dbg(de, tx_queued, dev, "tx queued, slot %d, skblen %d\n",
		  entry, skb->len);

	if (tx_free == 0)
		netif_stop_queue(dev);

	spin_unlock_irq(&de->lock);

	/* Trigger an immediate transmit demand. */
	dw32(TxPoll, NormalTxPoll);

	return NETDEV_TX_OK;
}

/* Set or clear the multicast filter for this adaptor.
   Note that we only use exclusion around actually queueing the
   new frame, not around filling de->setup_frame.  This is non-deterministic
   when re-entered but still correct. */

static void build_setup_frame_hash(u16 *setup_frm, struct net_device *dev)
{
	struct de_private *de = netdev_priv(dev);
	u16 hash_table[32];
	struct netdev_hw_addr *ha;
	int i;
	u16 *eaddrs;

	memset(hash_table, 0, sizeof(hash_table));
	__set_bit_le(255, hash_table);			/* Broadcast entry */
	/* This should work on big-endian machines as well. */
	netdev_for_each_mc_addr(ha, dev) {
		int index = ether_crc_le(ETH_ALEN, ha->addr) & 0x1ff;

		__set_bit_le(index, hash_table);
	}

	for (i = 0; i < 32; i++) {
		*setup_frm++ = hash_table[i];
		*setup_frm++ = hash_table[i];
	}
	setup_frm = &de->setup_frame[13*6];

	/* Fill the final entry with our physical address. */
	eaddrs = (u16 *)dev->dev_addr;
	*setup_frm++ = eaddrs[0]; *setup_frm++ = eaddrs[0];
	*setup_frm++ = eaddrs[1]; *setup_frm++ = eaddrs[1];
	*setup_frm++ = eaddrs[2]; *setup_frm++ = eaddrs[2];
}

static void build_setup_frame_perfect(u16 *setup_frm, struct net_device *dev)
{
	struct de_private *de = netdev_priv(dev);
	struct netdev_hw_addr *ha;
	u16 *eaddrs;

	/* We have <= 14 addresses so we can use the wonderful
	   16 address perfect filtering of the Tulip. */
	netdev_for_each_mc_addr(ha, dev) {
		eaddrs = (u16 *) ha->addr;
		*setup_frm++ = *eaddrs; *setup_frm++ = *eaddrs++;
		*setup_frm++ = *eaddrs; *setup_frm++ = *eaddrs++;
		*setup_frm++ = *eaddrs; *setup_frm++ = *eaddrs++;
	}
	/* Fill the unused entries with the broadcast address. */
	memset(setup_frm, 0xff, (15 - netdev_mc_count(dev)) * 12);
	setup_frm = &de->setup_frame[15*6];

	/* Fill the final entry with our physical address. */
	eaddrs = (u16 *)dev->dev_addr;
	*setup_frm++ = eaddrs[0]; *setup_frm++ = eaddrs[0];
	*setup_frm++ = eaddrs[1]; *setup_frm++ = eaddrs[1];
	*setup_frm++ = eaddrs[2]; *setup_frm++ = eaddrs[2];
}


static void __de_set_rx_mode (struct net_device *dev)
{
	struct de_private *de = netdev_priv(dev);
	u32 macmode;
	unsigned int entry;
	u32 mapping;
	struct de_desc *txd;
	struct de_desc *dummy_txd = NULL;

	macmode = dr32(MacMode) & ~(AcceptAllMulticast | AcceptAllPhys);

	if (dev->flags & IFF_PROMISC) {	/* Set promiscuous. */
		macmode |= AcceptAllMulticast | AcceptAllPhys;
		goto out;
	}

	if ((netdev_mc_count(dev) > 1000) || (dev->flags & IFF_ALLMULTI)) {
		/* Too many to filter well -- accept all multicasts. */
		macmode |= AcceptAllMulticast;
		goto out;
	}

	/* Note that only the low-address shortword of setup_frame is valid!
	   The values are doubled for big-endian architectures. */
	if (netdev_mc_count(dev) > 14)	/* Must use a multicast hash table. */
		build_setup_frame_hash (de->setup_frame, dev);
	else
		build_setup_frame_perfect (de->setup_frame, dev);

	/*
	 * Now add this frame to the Tx list.
	 */

	entry = de->tx_head;

	/* Avoid a chip errata by prefixing a dummy entry. */
	if (entry != 0) {
		de->tx_skb[entry].skb = DE_DUMMY_SKB;

		dummy_txd = &de->tx_ring[entry];
		dummy_txd->opts2 = (entry == (DE_TX_RING_SIZE - 1)) ?
				   cpu_to_le32(RingEnd) : 0;
		dummy_txd->addr1 = 0;

		/* Must set DescOwned later to avoid race with chip */

		entry = NEXT_TX(entry);
	}

	de->tx_skb[entry].skb = DE_SETUP_SKB;
	de->tx_skb[entry].mapping = mapping =
	    pci_map_single (de->pdev, de->setup_frame,
			    sizeof (de->setup_frame), PCI_DMA_TODEVICE);

	/* Put the setup frame on the Tx list. */
	txd = &de->tx_ring[entry];
	if (entry == (DE_TX_RING_SIZE - 1))
		txd->opts2 = cpu_to_le32(SetupFrame | RingEnd | sizeof (de->setup_frame));
	else
		txd->opts2 = cpu_to_le32(SetupFrame | sizeof (de->setup_frame));
	txd->addr1 = cpu_to_le32(mapping);
	wmb();

	txd->opts1 = cpu_to_le32(DescOwn);
	wmb();

	if (dummy_txd) {
		dummy_txd->opts1 = cpu_to_le32(DescOwn);
		wmb();
	}

	de->tx_head = NEXT_TX(entry);

	if (TX_BUFFS_AVAIL(de) == 0)
		netif_stop_queue(dev);

	/* Trigger an immediate transmit demand. */
	dw32(TxPoll, NormalTxPoll);

out:
	if (macmode != dr32(MacMode))
		dw32(MacMode, macmode);
}

static void de_set_rx_mode (struct net_device *dev)
{
	unsigned long flags;
	struct de_private *de = netdev_priv(dev);

	spin_lock_irqsave (&de->lock, flags);
	__de_set_rx_mode(dev);
	spin_unlock_irqrestore (&de->lock, flags);
}

static inline void de_rx_missed(struct de_private *de, u32 rx_missed)
{
	if (unlikely(rx_missed & RxMissedOver))
		de->dev->stats.rx_missed_errors += RxMissedMask;
	else
		de->dev->stats.rx_missed_errors += (rx_missed & RxMissedMask);
}

static void __de_get_stats(struct de_private *de)
{
	u32 tmp = dr32(RxMissed); /* self-clearing */

	de_rx_missed(de, tmp);
}

static struct net_device_stats *de_get_stats(struct net_device *dev)
{
	struct de_private *de = netdev_priv(dev);

	/* The chip only need report frame silently dropped. */
	spin_lock_irq(&de->lock);
 	if (netif_running(dev) && netif_device_present(dev))
 		__de_get_stats(de);
	spin_unlock_irq(&de->lock);

	return &dev->stats;
}

static inline int de_is_running (struct de_private *de)
{
	return (dr32(MacStatus) & (RxState | TxState)) ? 1 : 0;
}

static void de_stop_rxtx (struct de_private *de)
{
	u32 macmode;
	unsigned int i = 1300/100;

	macmode = dr32(MacMode);
	if (macmode & RxTx) {
		dw32(MacMode, macmode & ~RxTx);
		dr32(MacMode);
	}

	/* wait until in-flight frame completes.
	 * Max time @ 10BT: 1500*8b/10Mbps == 1200us (+ 100us margin)
	 * Typically expect this loop to end in < 50 us on 100BT.
	 */
	while (--i) {
		if (!de_is_running(de))
			return;
		udelay(100);
	}

	netdev_warn(de->dev, "timeout expired, stopping DMA\n");
}

static inline void de_start_rxtx (struct de_private *de)
{
	u32 macmode;

	macmode = dr32(MacMode);
	if ((macmode & RxTx) != RxTx) {
		dw32(MacMode, macmode | RxTx);
		dr32(MacMode);
	}
}

static void de_stop_hw (struct de_private *de)
{

	udelay(5);
	dw32(IntrMask, 0);

	de_stop_rxtx(de);

	dw32(MacStatus, dr32(MacStatus));

	udelay(10);

	de->rx_tail = 0;
	de->tx_head = de->tx_tail = 0;
}

static void de_link_up(struct de_private *de)
{
	if (!netif_carrier_ok(de->dev)) {
		netif_carrier_on(de->dev);
		netif_info(de, link, de->dev, "link up, media %s\n",
			   media_name[de->media_type]);
	}
}

static void de_link_down(struct de_private *de)
{
	if (netif_carrier_ok(de->dev)) {
		netif_carrier_off(de->dev);
		netif_info(de, link, de->dev, "link down\n");
	}
}

static void de_set_media (struct de_private *de)
{
	unsigned media = de->media_type;
	u32 macmode = dr32(MacMode);

	if (de_is_running(de))
		netdev_warn(de->dev, "chip is running while changing media!\n");

	if (de->de21040)
		dw32(CSR11, FULL_DUPLEX_MAGIC);
	dw32(CSR13, 0); /* Reset phy */
	dw32(CSR14, de->media[media].csr14);
	dw32(CSR15, de->media[media].csr15);
	dw32(CSR13, de->media[media].csr13);

	/* must delay 10ms before writing to other registers,
	 * especially CSR6
	 */
	mdelay(10);

	if (media == DE_MEDIA_TP_FD)
		macmode |= FullDuplex;
	else
		macmode &= ~FullDuplex;

	netif_info(de, link, de->dev, "set link %s\n", media_name[media]);
	netif_info(de, hw, de->dev, "mode 0x%x, sia 0x%x,0x%x,0x%x,0x%x\n",
		   dr32(MacMode), dr32(SIAStatus),
		   dr32(CSR13), dr32(CSR14), dr32(CSR15));
	netif_info(de, hw, de->dev, "set mode 0x%x, set sia 0x%x,0x%x,0x%x\n",
		   macmode, de->media[media].csr13,
		   de->media[media].csr14, de->media[media].csr15);
	if (macmode != dr32(MacMode))
		dw32(MacMode, macmode);
}

static void de_next_media (struct de_private *de, const u32 *media,
			   unsigned int n_media)
{
	unsigned int i;

	for (i = 0; i < n_media; i++) {
		if (de_ok_to_advertise(de, media[i])) {
			de->media_type = media[i];
			return;
		}
	}
}

static void de21040_media_timer (struct timer_list *t)
{
	struct de_private *de = from_timer(de, t, media_timer);
	struct net_device *dev = de->dev;
	u32 status = dr32(SIAStatus);
	unsigned int carrier;
	unsigned long flags;

	carrier = (status & NetCxnErr) ? 0 : 1;

	if (carrier) {
		if (de->media_type != DE_MEDIA_AUI && (status & LinkFailStatus))
			goto no_link_yet;

		de->media_timer.expires = jiffies + DE_TIMER_LINK;
		add_timer(&de->media_timer);
		if (!netif_carrier_ok(dev))
			de_link_up(de);
		else
			netif_info(de, timer, dev, "%s link ok, status %x\n",
				   media_name[de->media_type], status);
		return;
	}

	de_link_down(de);

	if (de->media_lock)
		return;

	if (de->media_type == DE_MEDIA_AUI) {
		static const u32 next_state = DE_MEDIA_TP;
		de_next_media(de, &next_state, 1);
	} else {
		static const u32 next_state = DE_MEDIA_AUI;
		de_next_media(de, &next_state, 1);
	}

	spin_lock_irqsave(&de->lock, flags);
	de_stop_rxtx(de);
	spin_unlock_irqrestore(&de->lock, flags);
	de_set_media(de);
	de_start_rxtx(de);

no_link_yet:
	de->media_timer.expires = jiffies + DE_TIMER_NO_LINK;
	add_timer(&de->media_timer);

	netif_info(de, timer, dev, "no link, trying media %s, status %x\n",
		   media_name[de->media_type], status);
}

static unsigned int de_ok_to_advertise (struct de_private *de, u32 new_media)
{
	switch (new_media) {
	case DE_MEDIA_TP_AUTO:
		if (!(de->media_advertise & ADVERTISED_Autoneg))
			return 0;
		if (!(de->media_advertise & (ADVERTISED_10baseT_Half | ADVERTISED_10baseT_Full)))
			return 0;
		break;
	case DE_MEDIA_BNC:
		if (!(de->media_advertise & ADVERTISED_BNC))
			return 0;
		break;
	case DE_MEDIA_AUI:
		if (!(de->media_advertise & ADVERTISED_AUI))
			return 0;
		break;
	case DE_MEDIA_TP:
		if (!(de->media_advertise & ADVERTISED_10baseT_Half))
			return 0;
		break;
	case DE_MEDIA_TP_FD:
		if (!(de->media_advertise & ADVERTISED_10baseT_Full))
			return 0;
		break;
	}

	return 1;
}

static void de21041_media_timer (struct timer_list *t)
{
	struct de_private *de = from_timer(de, t, media_timer);
	struct net_device *dev = de->dev;
	u32 status = dr32(SIAStatus);
	unsigned int carrier;
	unsigned long flags;

	/* clear port active bits */
	dw32(SIAStatus, NonselPortActive | SelPortActive);

	carrier = (status & NetCxnErr) ? 0 : 1;

	if (carrier) {
		if ((de->media_type == DE_MEDIA_TP_AUTO ||
		     de->media_type == DE_MEDIA_TP ||
		     de->media_type == DE_MEDIA_TP_FD) &&
		    (status & LinkFailStatus))
			goto no_link_yet;

		de->media_timer.expires = jiffies + DE_TIMER_LINK;
		add_timer(&de->media_timer);
		if (!netif_carrier_ok(dev))
			de_link_up(de);
		else
			netif_info(de, timer, dev,
				   "%s link ok, mode %x status %x\n",
				   media_name[de->media_type],
				   dr32(MacMode), status);
		return;
	}

	de_link_down(de);

	/* if media type locked, don't switch media */
	if (de->media_lock)
		goto set_media;

	/* if activity detected, use that as hint for new media type */
	if (status & NonselPortActive) {
		unsigned int have_media = 1;

		/* if AUI/BNC selected, then activity is on TP port */
		if (de->media_type == DE_MEDIA_AUI ||
		    de->media_type == DE_MEDIA_BNC) {
			if (de_ok_to_advertise(de, DE_MEDIA_TP_AUTO))
				de->media_type = DE_MEDIA_TP_AUTO;
			else
				have_media = 0;
		}

		/* TP selected.  If there is only TP and BNC, then it's BNC */
		else if (((de->media_supported & DE_AUI_BNC) == SUPPORTED_BNC) &&
			 de_ok_to_advertise(de, DE_MEDIA_BNC))
			de->media_type = DE_MEDIA_BNC;

		/* TP selected.  If there is only TP and AUI, then it's AUI */
		else if (((de->media_supported & DE_AUI_BNC) == SUPPORTED_AUI) &&
			 de_ok_to_advertise(de, DE_MEDIA_AUI))
			de->media_type = DE_MEDIA_AUI;

		/* otherwise, ignore the hint */
		else
			have_media = 0;

		if (have_media)
			goto set_media;
	}

	/*
	 * Absent or ambiguous activity hint, move to next advertised
	 * media state.  If de->media_type is left unchanged, this
	 * simply resets the PHY and reloads the current media settings.
	 */
	if (de->media_type == DE_MEDIA_AUI) {
		static const u32 next_states[] = {
			DE_MEDIA_BNC, DE_MEDIA_TP_AUTO
		};
		de_next_media(de, next_states, ARRAY_SIZE(next_states));
	} else if (de->media_type == DE_MEDIA_BNC) {
		static const u32 next_states[] = {
			DE_MEDIA_TP_AUTO, DE_MEDIA_AUI
		};
		de_next_media(de, next_states, ARRAY_SIZE(next_states));
	} else {
		static const u32 next_states[] = {
			DE_MEDIA_AUI, DE_MEDIA_BNC, DE_MEDIA_TP_AUTO
		};
		de_next_media(de, next_states, ARRAY_SIZE(next_states));
	}

set_media:
	spin_lock_irqsave(&de->lock, flags);
	de_stop_rxtx(de);
	spin_unlock_irqrestore(&de->lock, flags);
	de_set_media(de);
	de_start_rxtx(de);

no_link_yet:
	de->media_timer.expires = jiffies + DE_TIMER_NO_LINK;
	add_timer(&de->media_timer);

	netif_info(de, timer, dev, "no link, trying media %s, status %x\n",
		   media_name[de->media_type], status);
}

static void de_media_interrupt (struct de_private *de, u32 status)
{
	if (status & LinkPass) {
		/* Ignore if current media is AUI or BNC and we can't use TP */
		if ((de->media_type == DE_MEDIA_AUI ||
		     de->media_type == DE_MEDIA_BNC) &&
		    (de->media_lock ||
		     !de_ok_to_advertise(de, DE_MEDIA_TP_AUTO)))
			return;
		/* If current media is not TP, change it to TP */
		if ((de->media_type == DE_MEDIA_AUI ||
		     de->media_type == DE_MEDIA_BNC)) {
			de->media_type = DE_MEDIA_TP_AUTO;
			de_stop_rxtx(de);
			de_set_media(de);
			de_start_rxtx(de);
		}
		de_link_up(de);
		mod_timer(&de->media_timer, jiffies + DE_TIMER_LINK);
		return;
	}

	BUG_ON(!(status & LinkFail));
	/* Mark the link as down only if current media is TP */
	if (netif_carrier_ok(de->dev) && de->media_type != DE_MEDIA_AUI &&
	    de->media_type != DE_MEDIA_BNC) {
		de_link_down(de);
		mod_timer(&de->media_timer, jiffies + DE_TIMER_NO_LINK);
	}
}

static int de_reset_mac (struct de_private *de)
{
	u32 status, tmp;

	/*
	 * Reset MAC.  de4x5.c and tulip.c examined for "advice"
	 * in this area.
	 */

	if (dr32(BusMode) == 0xffffffff)
		return -EBUSY;

	/* Reset the chip, holding bit 0 set at least 50 PCI cycles. */
	dw32 (BusMode, CmdReset);
	mdelay (1);

	dw32 (BusMode, de_bus_mode);
	mdelay (1);

	for (tmp = 0; tmp < 5; tmp++) {
		dr32 (BusMode);
		mdelay (1);
	}

	mdelay (1);

	status = dr32(MacStatus);
	if (status & (RxState | TxState))
		return -EBUSY;
	if (status == 0xffffffff)
		return -ENODEV;
	return 0;
}

static void de_adapter_wake (struct de_private *de)
{
	u32 pmctl;

	if (de->de21040)
		return;

	pci_read_config_dword(de->pdev, PCIPM, &pmctl);
	if (pmctl & PM_Mask) {
		pmctl &= ~PM_Mask;
		pci_write_config_dword(de->pdev, PCIPM, pmctl);

		/* de4x5.c delays, so we do too */
		msleep(10);
	}
}

static void de_adapter_sleep (struct de_private *de)
{
	u32 pmctl;

	if (de->de21040)
		return;

	dw32(CSR13, 0); /* Reset phy */
	pci_read_config_dword(de->pdev, PCIPM, &pmctl);
	pmctl |= PM_Sleep;
	pci_write_config_dword(de->pdev, PCIPM, pmctl);
}

static int de_init_hw (struct de_private *de)
{
	struct net_device *dev = de->dev;
	u32 macmode;
	int rc;

	de_adapter_wake(de);

	macmode = dr32(MacMode) & ~MacModeClear;

	rc = de_reset_mac(de);
	if (rc)
		return rc;

	de_set_media(de); /* reset phy */

	dw32(RxRingAddr, de->ring_dma);
	dw32(TxRingAddr, de->ring_dma + (sizeof(struct de_desc) * DE_RX_RING_SIZE));

	dw32(MacMode, RxTx | macmode);

	dr32(RxMissed); /* self-clearing */

	dw32(IntrMask, de_intr_mask);

	de_set_rx_mode(dev);

	return 0;
}

static int de_refill_rx (struct de_private *de)
{
	unsigned i;

	for (i = 0; i < DE_RX_RING_SIZE; i++) {
		struct sk_buff *skb;

		skb = netdev_alloc_skb(de->dev, de->rx_buf_sz);
		if (!skb)
			goto err_out;

		de->rx_skb[i].mapping = pci_map_single(de->pdev,
			skb->data, de->rx_buf_sz, PCI_DMA_FROMDEVICE);
		de->rx_skb[i].skb = skb;

		de->rx_ring[i].opts1 = cpu_to_le32(DescOwn);
		if (i == (DE_RX_RING_SIZE - 1))
			de->rx_ring[i].opts2 =
				cpu_to_le32(RingEnd | de->rx_buf_sz);
		else
			de->rx_ring[i].opts2 = cpu_to_le32(de->rx_buf_sz);
		de->rx_ring[i].addr1 = cpu_to_le32(de->rx_skb[i].mapping);
		de->rx_ring[i].addr2 = 0;
	}

	return 0;

err_out:
	de_clean_rings(de);
	return -ENOMEM;
}

static int de_init_rings (struct de_private *de)
{
	memset(de->tx_ring, 0, sizeof(struct de_desc) * DE_TX_RING_SIZE);
	de->tx_ring[DE_TX_RING_SIZE - 1].opts2 = cpu_to_le32(RingEnd);

	de->rx_tail = 0;
	de->tx_head = de->tx_tail = 0;

	return de_refill_rx (de);
}

static int de_alloc_rings (struct de_private *de)
{
	de->rx_ring = pci_alloc_consistent(de->pdev, DE_RING_BYTES, &de->ring_dma);
	if (!de->rx_ring)
		return -ENOMEM;
	de->tx_ring = &de->rx_ring[DE_RX_RING_SIZE];
	return de_init_rings(de);
}

static void de_clean_rings (struct de_private *de)
{
	unsigned i;

	memset(de->rx_ring, 0, sizeof(struct de_desc) * DE_RX_RING_SIZE);
	de->rx_ring[DE_RX_RING_SIZE - 1].opts2 = cpu_to_le32(RingEnd);
	wmb();
	memset(de->tx_ring, 0, sizeof(struct de_desc) * DE_TX_RING_SIZE);
	de->tx_ring[DE_TX_RING_SIZE - 1].opts2 = cpu_to_le32(RingEnd);
	wmb();

	for (i = 0; i < DE_RX_RING_SIZE; i++) {
		if (de->rx_skb[i].skb) {
			pci_unmap_single(de->pdev, de->rx_skb[i].mapping,
					 de->rx_buf_sz, PCI_DMA_FROMDEVICE);
			dev_kfree_skb(de->rx_skb[i].skb);
		}
	}

	for (i = 0; i < DE_TX_RING_SIZE; i++) {
		struct sk_buff *skb = de->tx_skb[i].skb;
		if ((skb) && (skb != DE_DUMMY_SKB)) {
			if (skb != DE_SETUP_SKB) {
				de->dev->stats.tx_dropped++;
				pci_unmap_single(de->pdev,
					de->tx_skb[i].mapping,
					skb->len, PCI_DMA_TODEVICE);
				dev_kfree_skb(skb);
			} else {
				pci_unmap_single(de->pdev,
					de->tx_skb[i].mapping,
					sizeof(de->setup_frame),
					PCI_DMA_TODEVICE);
			}
		}
	}

	memset(&de->rx_skb, 0, sizeof(struct ring_info) * DE_RX_RING_SIZE);
	memset(&de->tx_skb, 0, sizeof(struct ring_info) * DE_TX_RING_SIZE);
}

static void de_free_rings (struct de_private *de)
{
	de_clean_rings(de);
	pci_free_consistent(de->pdev, DE_RING_BYTES, de->rx_ring, de->ring_dma);
	de->rx_ring = NULL;
	de->tx_ring = NULL;
}

static int de_open (struct net_device *dev)
{
	struct de_private *de = netdev_priv(dev);
	const int irq = de->pdev->irq;
	int rc;

	netif_dbg(de, ifup, dev, "enabling interface\n");

	de->rx_buf_sz = (dev->mtu <= 1500 ? PKT_BUF_SZ : dev->mtu + 32);

	rc = de_alloc_rings(de);
	if (rc) {
		netdev_err(dev, "ring allocation failure, err=%d\n", rc);
		return rc;
	}

	dw32(IntrMask, 0);

	rc = request_irq(irq, de_interrupt, IRQF_SHARED, dev->name, dev);
	if (rc) {
		netdev_err(dev, "IRQ %d request failure, err=%d\n", irq, rc);
		goto err_out_free;
	}

	rc = de_init_hw(de);
	if (rc) {
		netdev_err(dev, "h/w init failure, err=%d\n", rc);
		goto err_out_free_irq;
	}

	netif_start_queue(dev);
	mod_timer(&de->media_timer, jiffies + DE_TIMER_NO_LINK);

	return 0;

err_out_free_irq:
	free_irq(irq, dev);
err_out_free:
	de_free_rings(de);
	return rc;
}

static int de_close (struct net_device *dev)
{
	struct de_private *de = netdev_priv(dev);
	unsigned long flags;

	netif_dbg(de, ifdown, dev, "disabling interface\n");

	del_timer_sync(&de->media_timer);

	spin_lock_irqsave(&de->lock, flags);
	de_stop_hw(de);
	netif_stop_queue(dev);
	netif_carrier_off(dev);
	spin_unlock_irqrestore(&de->lock, flags);

	free_irq(de->pdev->irq, dev);

	de_free_rings(de);
	de_adapter_sleep(de);
	return 0;
}

static void de_tx_timeout (struct net_device *dev)
{
	struct de_private *de = netdev_priv(dev);
	const int irq = de->pdev->irq;

	netdev_dbg(dev, "NIC status %08x mode %08x sia %08x desc %u/%u/%u\n",
		   dr32(MacStatus), dr32(MacMode), dr32(SIAStatus),
		   de->rx_tail, de->tx_head, de->tx_tail);

	del_timer_sync(&de->media_timer);

	disable_irq(irq);
	spin_lock_irq(&de->lock);

	de_stop_hw(de);
	netif_stop_queue(dev);
	netif_carrier_off(dev);

	spin_unlock_irq(&de->lock);
	enable_irq(irq);

	/* Update the error counts. */
	__de_get_stats(de);

	synchronize_irq(irq);
	de_clean_rings(de);

	de_init_rings(de);

	de_init_hw(de);

	netif_wake_queue(dev);
}

static void __de_get_regs(struct de_private *de, u8 *buf)
{
	int i;
	u32 *rbuf = (u32 *)buf;

	/* read all CSRs */
	for (i = 0; i < DE_NUM_REGS; i++)
		rbuf[i] = dr32(i * 8);

	/* handle self-clearing RxMissed counter, CSR8 */
	de_rx_missed(de, rbuf[8]);
}

static void __de_get_link_ksettings(struct de_private *de,
				    struct ethtool_link_ksettings *cmd)
{
	ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.supported,
						de->media_supported);
	cmd->base.phy_address = 0;
	ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.advertising,
						de->media_advertise);

	switch (de->media_type) {
	case DE_MEDIA_AUI:
		cmd->base.port = PORT_AUI;
		break;
	case DE_MEDIA_BNC:
		cmd->base.port = PORT_BNC;
		break;
	default:
		cmd->base.port = PORT_TP;
		break;
	}

	cmd->base.speed = 10;

	if (dr32(MacMode) & FullDuplex)
		cmd->base.duplex = DUPLEX_FULL;
	else
		cmd->base.duplex = DUPLEX_HALF;

	if (de->media_lock)
		cmd->base.autoneg = AUTONEG_DISABLE;
	else
		cmd->base.autoneg = AUTONEG_ENABLE;

	/* ignore maxtxpkt, maxrxpkt for now */
}

static int __de_set_link_ksettings(struct de_private *de,
				   const struct ethtool_link_ksettings *cmd)
{
	u32 new_media;
	unsigned int media_lock;
	u8 duplex = cmd->base.duplex;
	u8 port = cmd->base.port;
	u8 autoneg = cmd->base.autoneg;
	u32 advertising;

	ethtool_convert_link_mode_to_legacy_u32(&advertising,
						cmd->link_modes.advertising);

	if (cmd->base.speed != 10)
		return -EINVAL;
	if (duplex != DUPLEX_HALF && duplex != DUPLEX_FULL)
		return -EINVAL;
	if (port != PORT_TP && port != PORT_AUI && port != PORT_BNC)
		return -EINVAL;
	if (de->de21040 && port == PORT_BNC)
		return -EINVAL;
	if (autoneg != AUTONEG_DISABLE && autoneg != AUTONEG_ENABLE)
		return -EINVAL;
	if (advertising & ~de->media_supported)
		return -EINVAL;
	if (autoneg == AUTONEG_ENABLE &&
	    (!(advertising & ADVERTISED_Autoneg)))
		return -EINVAL;

	switch (port) {
	case PORT_AUI:
		new_media = DE_MEDIA_AUI;
		if (!(advertising & ADVERTISED_AUI))
			return -EINVAL;
		break;
	case PORT_BNC:
		new_media = DE_MEDIA_BNC;
		if (!(advertising & ADVERTISED_BNC))
			return -EINVAL;
		break;
	default:
		if (autoneg == AUTONEG_ENABLE)
			new_media = DE_MEDIA_TP_AUTO;
		else if (duplex == DUPLEX_FULL)
			new_media = DE_MEDIA_TP_FD;
		else
			new_media = DE_MEDIA_TP;
		if (!(advertising & ADVERTISED_TP))
			return -EINVAL;
		if (!(advertising & (ADVERTISED_10baseT_Full |
				     ADVERTISED_10baseT_Half)))
			return -EINVAL;
		break;
	}

	media_lock = (autoneg == AUTONEG_ENABLE) ? 0 : 1;

	if ((new_media == de->media_type) &&
	    (media_lock == de->media_lock) &&
	    (advertising == de->media_advertise))
		return 0; /* nothing to change */

	de_link_down(de);
	mod_timer(&de->media_timer, jiffies + DE_TIMER_NO_LINK);
	de_stop_rxtx(de);

	de->media_type = new_media;
	de->media_lock = media_lock;
	de->media_advertise = advertising;
	de_set_media(de);
	if (netif_running(de->dev))
		de_start_rxtx(de);

	return 0;
}

static void de_get_drvinfo (struct net_device *dev,struct ethtool_drvinfo *info)
{
	struct de_private *de = netdev_priv(dev);

	strlcpy(info->driver, DRV_NAME, sizeof(info->driver));
	strlcpy(info->version, DRV_VERSION, sizeof(info->version));
	strlcpy(info->bus_info, pci_name(de->pdev), sizeof(info->bus_info));
}

static int de_get_regs_len(struct net_device *dev)
{
	return DE_REGS_SIZE;
}

static int de_get_link_ksettings(struct net_device *dev,
				 struct ethtool_link_ksettings *cmd)
{
	struct de_private *de = netdev_priv(dev);

	spin_lock_irq(&de->lock);
	__de_get_link_ksettings(de, cmd);
	spin_unlock_irq(&de->lock);

	return 0;
}

static int de_set_link_ksettings(struct net_device *dev,
				 const struct ethtool_link_ksettings *cmd)
{
	struct de_private *de = netdev_priv(dev);
	int rc;

	spin_lock_irq(&de->lock);
	rc = __de_set_link_ksettings(de, cmd);
	spin_unlock_irq(&de->lock);

	return rc;
}

static u32 de_get_msglevel(struct net_device *dev)
{
	struct de_private *de = netdev_priv(dev);

	return de->msg_enable;
}

static void de_set_msglevel(struct net_device *dev, u32 msglvl)
{
	struct de_private *de = netdev_priv(dev);

	de->msg_enable = msglvl;
}

static int de_get_eeprom(struct net_device *dev,
			 struct ethtool_eeprom *eeprom, u8 *data)
{
	struct de_private *de = netdev_priv(dev);

	if (!de->ee_data)
		return -EOPNOTSUPP;
	if ((eeprom->offset != 0) || (eeprom->magic != 0) ||
	    (eeprom->len != DE_EEPROM_SIZE))
		return -EINVAL;
	memcpy(data, de->ee_data, eeprom->len);

	return 0;
}

static int de_nway_reset(struct net_device *dev)
{
	struct de_private *de = netdev_priv(dev);
	u32 status;

	if (de->media_type != DE_MEDIA_TP_AUTO)
		return -EINVAL;
	if (netif_carrier_ok(de->dev))
		de_link_down(de);

	status = dr32(SIAStatus);
	dw32(SIAStatus, (status & ~NWayState) | NWayRestart);
	netif_info(de, link, dev, "link nway restart, status %x,%x\n",
		   status, dr32(SIAStatus));
	return 0;
}

static void de_get_regs(struct net_device *dev, struct ethtool_regs *regs,
			void *data)
{
	struct de_private *de = netdev_priv(dev);

	regs->version = (DE_REGS_VER << 2) | de->de21040;

	spin_lock_irq(&de->lock);
	__de_get_regs(de, data);
	spin_unlock_irq(&de->lock);
}

static const struct ethtool_ops de_ethtool_ops = {
	.get_link		= ethtool_op_get_link,
	.get_drvinfo		= de_get_drvinfo,
	.get_regs_len		= de_get_regs_len,
	.get_msglevel		= de_get_msglevel,
	.set_msglevel		= de_set_msglevel,
	.get_eeprom		= de_get_eeprom,
	.nway_reset		= de_nway_reset,
	.get_regs		= de_get_regs,
	.get_link_ksettings	= de_get_link_ksettings,
	.set_link_ksettings	= de_set_link_ksettings,
};

static void de21040_get_mac_address(struct de_private *de)
{
	unsigned i;

	dw32 (ROMCmd, 0);	/* Reset the pointer with a dummy write. */
	udelay(5);

	for (i = 0; i < 6; i++) {
		int value, boguscnt = 100000;
		do {
			value = dr32(ROMCmd);
			rmb();
		} while (value < 0 && --boguscnt > 0);
		de->dev->dev_addr[i] = value;
		udelay(1);
		if (boguscnt <= 0)
			pr_warn("timeout reading 21040 MAC address byte %u\n",
				i);
	}
}

static void de21040_get_media_info(struct de_private *de)
{
	unsigned int i;

	de->media_type = DE_MEDIA_TP;
	de->media_supported |= SUPPORTED_TP | SUPPORTED_10baseT_Full |
			       SUPPORTED_10baseT_Half | SUPPORTED_AUI;
	de->media_advertise = de->media_supported;

	for (i = 0; i < DE_MAX_MEDIA; i++) {
		switch (i) {
		case DE_MEDIA_AUI:
		case DE_MEDIA_TP:
		case DE_MEDIA_TP_FD:
			de->media[i].type = i;
			de->media[i].csr13 = t21040_csr13[i];
			de->media[i].csr14 = t21040_csr14[i];
			de->media[i].csr15 = t21040_csr15[i];
			break;
		default:
			de->media[i].type = DE_MEDIA_INVALID;
			break;
		}
	}
}

/* Note: this routine returns extra data bits for size detection. */
static unsigned tulip_read_eeprom(void __iomem *regs, int location,
				  int addr_len)
{
	int i;
	unsigned retval = 0;
	void __iomem *ee_addr = regs + ROMCmd;
	int read_cmd = location | (EE_READ_CMD << addr_len);

	writel(EE_ENB & ~EE_CS, ee_addr);
	writel(EE_ENB, ee_addr);

	/* Shift the read command bits out. */
	for (i = 4 + addr_len; i >= 0; i--) {
		short dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
		writel(EE_ENB | dataval, ee_addr);
		readl(ee_addr);
		writel(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
		readl(ee_addr);
		retval = (retval << 1) | ((readl(ee_addr) & EE_DATA_READ) ? 1 : 0);
	}
	writel(EE_ENB, ee_addr);
	readl(ee_addr);

	for (i = 16; i > 0; i--) {
		writel(EE_ENB | EE_SHIFT_CLK, ee_addr);
		readl(ee_addr);
		retval = (retval << 1) | ((readl(ee_addr) & EE_DATA_READ) ? 1 : 0);
		writel(EE_ENB, ee_addr);
		readl(ee_addr);
	}

	/* Terminate the EEPROM access. */
	writel(EE_ENB & ~EE_CS, ee_addr);
	return retval;
}

static void de21041_get_srom_info(struct de_private *de)
{
	unsigned i, sa_offset = 0, ofs;
	u8 ee_data[DE_EEPROM_SIZE + 6] = {};
	unsigned ee_addr_size = tulip_read_eeprom(de->regs, 0xff, 8) & 0x40000 ? 8 : 6;
	struct de_srom_info_leaf *il;
	void *bufp;

	/* download entire eeprom */
	for (i = 0; i < DE_EEPROM_WORDS; i++)
		((__le16 *)ee_data)[i] =
			cpu_to_le16(tulip_read_eeprom(de->regs, i, ee_addr_size));

	/* DEC now has a specification but early board makers
	   just put the address in the first EEPROM locations. */
	/* This does  memcmp(eedata, eedata+16, 8) */

#ifndef CONFIG_MIPS_COBALT

	for (i = 0; i < 8; i ++)
		if (ee_data[i] != ee_data[16+i])
			sa_offset = 20;

#endif

	/* store MAC address */
	for (i = 0; i < 6; i ++)
		de->dev->dev_addr[i] = ee_data[i + sa_offset];

	/* get offset of controller 0 info leaf.  ignore 2nd byte. */
	ofs = ee_data[SROMC0InfoLeaf];
	if (ofs >= (sizeof(ee_data) - sizeof(struct de_srom_info_leaf) - sizeof(struct de_srom_media_block)))
		goto bad_srom;

	/* get pointer to info leaf */
	il = (struct de_srom_info_leaf *) &ee_data[ofs];

	/* paranoia checks */
	if (il->n_blocks == 0)
		goto bad_srom;
	if ((sizeof(ee_data) - ofs) <
	    (sizeof(struct de_srom_info_leaf) + (sizeof(struct de_srom_media_block) * il->n_blocks)))
		goto bad_srom;

	/* get default media type */
	switch (get_unaligned(&il->default_media)) {
	case 0x0001:  de->media_type = DE_MEDIA_BNC; break;
	case 0x0002:  de->media_type = DE_MEDIA_AUI; break;
	case 0x0204:  de->media_type = DE_MEDIA_TP_FD; break;
	default: de->media_type = DE_MEDIA_TP_AUTO; break;
	}

	if (netif_msg_probe(de))
		pr_info("de%d: SROM leaf offset %u, default media %s\n",
		       de->board_idx, ofs, media_name[de->media_type]);

	/* init SIA register values to defaults */
	for (i = 0; i < DE_MAX_MEDIA; i++) {
		de->media[i].type = DE_MEDIA_INVALID;
		de->media[i].csr13 = 0xffff;
		de->media[i].csr14 = 0xffff;
		de->media[i].csr15 = 0xffff;
	}

	/* parse media blocks to see what medias are supported,
	 * and if any custom CSR values are provided
	 */
	bufp = ((void *)il) + sizeof(*il);
	for (i = 0; i < il->n_blocks; i++) {
		struct de_srom_media_block *ib = bufp;
		unsigned idx;

		/* index based on media type in media block */
		switch(ib->opts & MediaBlockMask) {
		case 0: /* 10baseT */
			de->media_supported |= SUPPORTED_TP | SUPPORTED_10baseT_Half
					  | SUPPORTED_Autoneg;
			idx = DE_MEDIA_TP;
			de->media[DE_MEDIA_TP_AUTO].type = DE_MEDIA_TP_AUTO;
			break;
		case 1: /* BNC */
			de->media_supported |= SUPPORTED_BNC;
			idx = DE_MEDIA_BNC;
			break;
		case 2: /* AUI */
			de->media_supported |= SUPPORTED_AUI;
			idx = DE_MEDIA_AUI;
			break;
		case 4: /* 10baseT-FD */
			de->media_supported |= SUPPORTED_TP | SUPPORTED_10baseT_Full
					  | SUPPORTED_Autoneg;
			idx = DE_MEDIA_TP_FD;
			de->media[DE_MEDIA_TP_AUTO].type = DE_MEDIA_TP_AUTO;
			break;
		default:
			goto bad_srom;
		}

		de->media[idx].type = idx;

		if (netif_msg_probe(de))
			pr_info("de%d:   media block #%u: %s",
				de->board_idx, i,
				media_name[de->media[idx].type]);

		bufp += sizeof (ib->opts);

		if (ib->opts & MediaCustomCSRs) {
			de->media[idx].csr13 = get_unaligned(&ib->csr13);
			de->media[idx].csr14 = get_unaligned(&ib->csr14);
			de->media[idx].csr15 = get_unaligned(&ib->csr15);
			bufp += sizeof(ib->csr13) + sizeof(ib->csr14) +
				sizeof(ib->csr15);

			if (netif_msg_probe(de))
				pr_cont(" (%x,%x,%x)\n",
					de->media[idx].csr13,
					de->media[idx].csr14,
					de->media[idx].csr15);

		} else {
			if (netif_msg_probe(de))
				pr_cont("\n");
		}

		if (bufp > ((void *)&ee_data[DE_EEPROM_SIZE - 3]))
			break;
	}

	de->media_advertise = de->media_supported;

fill_defaults:
	/* fill in defaults, for cases where custom CSRs not used */
	for (i = 0; i < DE_MAX_MEDIA; i++) {
		if (de->media[i].csr13 == 0xffff)
			de->media[i].csr13 = t21041_csr13[i];
		if (de->media[i].csr14 == 0xffff) {
			/* autonegotiation is broken at least on some chip
			   revisions - rev. 0x21 works, 0x11 does not */
			if (de->pdev->revision < 0x20)
				de->media[i].csr14 = t21041_csr14_brk[i];
			else
				de->media[i].csr14 = t21041_csr14[i];
		}
		if (de->media[i].csr15 == 0xffff)
			de->media[i].csr15 = t21041_csr15[i];
	}

	de->ee_data = kmemdup(&ee_data[0], DE_EEPROM_SIZE, GFP_KERNEL);

	return;

bad_srom:
	/* for error cases, it's ok to assume we support all these */
	for (i = 0; i < DE_MAX_MEDIA; i++)
		de->media[i].type = i;
	de->media_supported =
		SUPPORTED_10baseT_Half |
		SUPPORTED_10baseT_Full |
		SUPPORTED_Autoneg |
		SUPPORTED_TP |
		SUPPORTED_AUI |
		SUPPORTED_BNC;
	goto fill_defaults;
}

static const struct net_device_ops de_netdev_ops = {
	.ndo_open		= de_open,
	.ndo_stop		= de_close,
	.ndo_set_rx_mode	= de_set_rx_mode,
	.ndo_start_xmit		= de_start_xmit,
	.ndo_get_stats		= de_get_stats,
	.ndo_tx_timeout 	= de_tx_timeout,
	.ndo_set_mac_address 	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};

static int de_init_one(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct net_device *dev;
	struct de_private *de;
	int rc;
	void __iomem *regs;
	unsigned long pciaddr;
	static int board_idx = -1;

	board_idx++;

#ifndef MODULE
	if (board_idx == 0)
		pr_info("%s\n", version);
#endif

	/* allocate a new ethernet device structure, and fill in defaults */
	dev = alloc_etherdev(sizeof(struct de_private));
	if (!dev)
		return -ENOMEM;

	dev->netdev_ops = &de_netdev_ops;
	SET_NETDEV_DEV(dev, &pdev->dev);
	dev->ethtool_ops = &de_ethtool_ops;
	dev->watchdog_timeo = TX_TIMEOUT;

	de = netdev_priv(dev);
	de->de21040 = ent->driver_data == 0 ? 1 : 0;
	de->pdev = pdev;
	de->dev = dev;
	de->msg_enable = (debug < 0 ? DE_DEF_MSG_ENABLE : debug);
	de->board_idx = board_idx;
	spin_lock_init (&de->lock);
	timer_setup(&de->media_timer,
		    de->de21040 ? de21040_media_timer : de21041_media_timer,
		    0);

	netif_carrier_off(dev);

	/* wake up device, assign resources */
	rc = pci_enable_device(pdev);
	if (rc)
		goto err_out_free;

	/* reserve PCI resources to ensure driver atomicity */
	rc = pci_request_regions(pdev, DRV_NAME);
	if (rc)
		goto err_out_disable;

	/* check for invalid IRQ value */
	if (pdev->irq < 2) {
		rc = -EIO;
		pr_err("invalid irq (%d) for pci dev %s\n",
		       pdev->irq, pci_name(pdev));
		goto err_out_res;
	}

	/* obtain and check validity of PCI I/O address */
	pciaddr = pci_resource_start(pdev, 1);
	if (!pciaddr) {
		rc = -EIO;
		pr_err("no MMIO resource for pci dev %s\n", pci_name(pdev));
		goto err_out_res;
	}
	if (pci_resource_len(pdev, 1) < DE_REGS_SIZE) {
		rc = -EIO;
		pr_err("MMIO resource (%llx) too small on pci dev %s\n",
		       (unsigned long long)pci_resource_len(pdev, 1),
		       pci_name(pdev));
		goto err_out_res;
	}

	/* remap CSR registers */
	regs = ioremap_nocache(pciaddr, DE_REGS_SIZE);
	if (!regs) {
		rc = -EIO;
		pr_err("Cannot map PCI MMIO (%llx@%lx) on pci dev %s\n",
		       (unsigned long long)pci_resource_len(pdev, 1),
		       pciaddr, pci_name(pdev));
		goto err_out_res;
	}
	de->regs = regs;

	de_adapter_wake(de);

	/* make sure hardware is not running */
	rc = de_reset_mac(de);
	if (rc) {
		pr_err("Cannot reset MAC, pci dev %s\n", pci_name(pdev));
		goto err_out_iomap;
	}

	/* get MAC address, initialize default media type and
	 * get list of supported media
	 */
	if (de->de21040) {
		de21040_get_mac_address(de);
		de21040_get_media_info(de);
	} else {
		de21041_get_srom_info(de);
	}

	/* register new network interface with kernel */
	rc = register_netdev(dev);
	if (rc)
		goto err_out_iomap;

	/* print info about board and interface just registered */
	netdev_info(dev, "%s at %p, %pM, IRQ %d\n",
		    de->de21040 ? "21040" : "21041",
		    regs, dev->dev_addr, pdev->irq);

	pci_set_drvdata(pdev, dev);

	/* enable busmastering */
	pci_set_master(pdev);

	/* put adapter to sleep */
	de_adapter_sleep(de);

	return 0;

err_out_iomap:
	kfree(de->ee_data);
	iounmap(regs);
err_out_res:
	pci_release_regions(pdev);
err_out_disable:
	pci_disable_device(pdev);
err_out_free:
	free_netdev(dev);
	return rc;
}

static void de_remove_one(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct de_private *de = netdev_priv(dev);

	BUG_ON(!dev);
	unregister_netdev(dev);
	kfree(de->ee_data);
	iounmap(de->regs);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
	free_netdev(dev);
}

#ifdef CONFIG_PM

static int de_suspend (struct pci_dev *pdev, pm_message_t state)
{
	struct net_device *dev = pci_get_drvdata (pdev);
	struct de_private *de = netdev_priv(dev);

	rtnl_lock();
	if (netif_running (dev)) {
		const int irq = pdev->irq;

		del_timer_sync(&de->media_timer);

		disable_irq(irq);
		spin_lock_irq(&de->lock);

		de_stop_hw(de);
		netif_stop_queue(dev);
		netif_device_detach(dev);
		netif_carrier_off(dev);

		spin_unlock_irq(&de->lock);
		enable_irq(irq);

		/* Update the error counts. */
		__de_get_stats(de);

		synchronize_irq(irq);
		de_clean_rings(de);

		de_adapter_sleep(de);
		pci_disable_device(pdev);
	} else {
		netif_device_detach(dev);
	}
	rtnl_unlock();
	return 0;
}

static int de_resume (struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata (pdev);
	struct de_private *de = netdev_priv(dev);
	int retval = 0;

	rtnl_lock();
	if (netif_device_present(dev))
		goto out;
	if (!netif_running(dev))
		goto out_attach;
	if ((retval = pci_enable_device(pdev))) {
		netdev_err(dev, "pci_enable_device failed in resume\n");
		goto out;
	}
	pci_set_master(pdev);
	de_init_rings(de);
	de_init_hw(de);
out_attach:
	netif_device_attach(dev);
out:
	rtnl_unlock();
	return 0;
}

#endif /* CONFIG_PM */

static struct pci_driver de_driver = {
	.name		= DRV_NAME,
	.id_table	= de_pci_tbl,
	.probe		= de_init_one,
	.remove		= de_remove_one,
#ifdef CONFIG_PM
	.suspend	= de_suspend,
	.resume		= de_resume,
#endif
};

static int __init de_init (void)
{
#ifdef MODULE
	pr_info("%s\n", version);
#endif
	return pci_register_driver(&de_driver);
}

static void __exit de_exit (void)
{
	pci_unregister_driver (&de_driver);
}

module_init(de_init);
module_exit(de_exit);