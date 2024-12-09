		if (skb) {
			dev->stats.tx_packets++;
			dev->stats.tx_bytes += skb->len;
			dev_kfree_skb_irq(skb);
			info->skb = NULL;
		}

		idx = (idx + 1) % ACE_TX_RING_ENTRIES(ap);
	} while (idx != txcsm);

	if (netif_queue_stopped(dev))
		netif_wake_queue(dev);

	wmb();
	ap->tx_ret_csm = txcsm;

	/* So... tx_ret_csm is advanced _after_ check for device wakeup.
	 *
	 * We could try to make it before. In this case we would get
	 * the following race condition: hard_start_xmit on other cpu
	 * enters after we advanced tx_ret_csm and fills space,
	 * which we have just freed, so that we make illegal device wakeup.
	 * There is no good way to workaround this (at entry
	 * to ace_start_xmit detects this condition and prevents
	 * ring corruption, but it is not a good workaround.)
	 *
	 * When tx_ret_csm is advanced after, we wake up device _only_
	 * if we really have some space in ring (though the core doing
	 * hard_start_xmit can see full ring for some period and has to
	 * synchronize.) Superb.
	 * BUT! We get another subtle race condition. hard_start_xmit
	 * may think that ring is full between wakeup and advancing
	 * tx_ret_csm and will stop device instantly! It is not so bad.
	 * We are guaranteed that there is something in ring, so that
	 * the next irq will resume transmission. To speedup this we could
	 * mark descriptor, which closes ring with BD_FLG_COAL_NOW
	 * (see ace_start_xmit).
	 *
	 * Well, this dilemma exists in all lock-free devices.
	 * We, following scheme used in drivers by Donald Becker,
	 * select the least dangerous.
	 *							--ANK
	 */
}


static irqreturn_t ace_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *)dev_id;
	struct ace_private *ap = netdev_priv(dev);
	struct ace_regs __iomem *regs = ap->regs;
	u32 idx;
	u32 txcsm, rxretcsm, rxretprd;
	u32 evtcsm, evtprd;

	/*
	 * In case of PCI shared interrupts or spurious interrupts,
	 * we want to make sure it is actually our interrupt before
	 * spending any time in here.
	 */
	if (!(readl(&regs->HostCtrl) & IN_INT))
		return IRQ_NONE;

	/*
	 * ACK intr now. Otherwise we will lose updates to rx_ret_prd,
	 * which happened _after_ rxretprd = *ap->rx_ret_prd; but before
	 * writel(0, &regs->Mb0Lo).
	 *
	 * "IRQ avoidance" recommended in docs applies to IRQs served
	 * threads and it is wrong even for that case.
	 */
	writel(0, &regs->Mb0Lo);
	readl(&regs->Mb0Lo);

	/*
	 * There is no conflict between transmit handling in
	 * start_xmit and receive processing, thus there is no reason
	 * to take a spin lock for RX handling. Wait until we start
	 * working on the other stuff - hey we don't need a spin lock
	 * anymore.
	 */
	rxretprd = *ap->rx_ret_prd;
	rxretcsm = ap->cur_rx;

	if (rxretprd != rxretcsm)
		ace_rx_int(dev, rxretprd, rxretcsm);

	txcsm = *ap->tx_csm;
	idx = ap->tx_ret_csm;

	if (txcsm != idx) {
		/*
		 * If each skb takes only one descriptor this check degenerates
		 * to identity, because new space has just been opened.
		 * But if skbs are fragmented we must check that this index
		 * update releases enough of space, otherwise we just
		 * wait for device to make more work.
		 */
		if (!tx_ring_full(ap, txcsm, ap->tx_prd))
			ace_tx_int(dev, txcsm, idx);
	}

	evtcsm = readl(&regs->EvtCsm);
	evtprd = *ap->evt_prd;

	if (evtcsm != evtprd) {
		evtcsm = ace_handle_event(dev, evtcsm, evtprd);
		writel(evtcsm, &regs->EvtCsm);
	}

	/*
	 * This has to go last in the interrupt handler and run with
	 * the spin lock released ... what lock?
	 */
	if (netif_running(dev)) {
		int cur_size;
		int run_tasklet = 0;

		cur_size = atomic_read(&ap->cur_rx_bufs);
		if (cur_size < RX_LOW_STD_THRES) {
			if ((cur_size < RX_PANIC_STD_THRES) &&
			    !test_and_set_bit(0, &ap->std_refill_busy)) {
#ifdef DEBUG
				printk("low on std buffers %i\n", cur_size);
#endif
				ace_load_std_rx_ring(dev,
						     RX_RING_SIZE - cur_size);
			} else
				run_tasklet = 1;
		}

		if (!ACE_IS_TIGON_I(ap)) {
			cur_size = atomic_read(&ap->cur_mini_bufs);
			if (cur_size < RX_LOW_MINI_THRES) {
				if ((cur_size < RX_PANIC_MINI_THRES) &&
				    !test_and_set_bit(0,
						      &ap->mini_refill_busy)) {
#ifdef DEBUG
					printk("low on mini buffers %i\n",
					       cur_size);
#endif
					ace_load_mini_rx_ring(dev,
							      RX_MINI_SIZE - cur_size);
				} else
					run_tasklet = 1;
			}
		}

		if (ap->jumbo) {
			cur_size = atomic_read(&ap->cur_jumbo_bufs);
			if (cur_size < RX_LOW_JUMBO_THRES) {
				if ((cur_size < RX_PANIC_JUMBO_THRES) &&
				    !test_and_set_bit(0,
						      &ap->jumbo_refill_busy)){
#ifdef DEBUG
					printk("low on jumbo buffers %i\n",
					       cur_size);
#endif
					ace_load_jumbo_rx_ring(dev,
							       RX_JUMBO_SIZE - cur_size);
				} else
					run_tasklet = 1;
			}
		}
		if (run_tasklet && !ap->tasklet_pending) {
			ap->tasklet_pending = 1;
			tasklet_schedule(&ap->ace_tasklet);
		}
	}

	return IRQ_HANDLED;
}

static int ace_open(struct net_device *dev)
{
	struct ace_private *ap = netdev_priv(dev);
	struct ace_regs __iomem *regs = ap->regs;
	struct cmd cmd;

	if (!(ap->fw_running)) {
		printk(KERN_WARNING "%s: Firmware not running!\n", dev->name);
		return -EBUSY;
	}

	writel(dev->mtu + ETH_HLEN + 4, &regs->IfMtu);

	cmd.evt = C_CLEAR_STATS;
	cmd.code = 0;
	cmd.idx = 0;
	ace_issue_cmd(regs, &cmd);

	cmd.evt = C_HOST_STATE;
	cmd.code = C_C_STACK_UP;
	cmd.idx = 0;
	ace_issue_cmd(regs, &cmd);

	if (ap->jumbo &&
	    !test_and_set_bit(0, &ap->jumbo_refill_busy))
		ace_load_jumbo_rx_ring(dev, RX_JUMBO_SIZE);

	if (dev->flags & IFF_PROMISC) {
		cmd.evt = C_SET_PROMISC_MODE;
		cmd.code = C_C_PROMISC_ENABLE;
		cmd.idx = 0;
		ace_issue_cmd(regs, &cmd);

		ap->promisc = 1;
	}else
		ap->promisc = 0;
	ap->mcast_all = 0;

#if 0
	cmd.evt = C_LNK_NEGOTIATION;
	cmd.code = 0;
	cmd.idx = 0;
	ace_issue_cmd(regs, &cmd);
#endif

	netif_start_queue(dev);

	/*
	 * Setup the bottom half rx ring refill handler
	 */
	tasklet_init(&ap->ace_tasklet, ace_tasklet, (unsigned long)dev);
	return 0;
}


static int ace_close(struct net_device *dev)
{
	struct ace_private *ap = netdev_priv(dev);
	struct ace_regs __iomem *regs = ap->regs;
	struct cmd cmd;
	unsigned long flags;
	short i;

	/*
	 * Without (or before) releasing irq and stopping hardware, this
	 * is an absolute non-sense, by the way. It will be reset instantly
	 * by the first irq.
	 */
	netif_stop_queue(dev);


	if (ap->promisc) {
		cmd.evt = C_SET_PROMISC_MODE;
		cmd.code = C_C_PROMISC_DISABLE;
		cmd.idx = 0;
		ace_issue_cmd(regs, &cmd);
		ap->promisc = 0;
	}

	cmd.evt = C_HOST_STATE;
	cmd.code = C_C_STACK_DOWN;
	cmd.idx = 0;
	ace_issue_cmd(regs, &cmd);

	tasklet_kill(&ap->ace_tasklet);

	/*
	 * Make sure one CPU is not processing packets while
	 * buffers are being released by another.
	 */

	local_irq_save(flags);
	ace_mask_irq(dev);

	for (i = 0; i < ACE_TX_RING_ENTRIES(ap); i++) {
		struct sk_buff *skb;
		struct tx_ring_info *info;

		info = ap->skb->tx_skbuff + i;
		skb = info->skb;

		if (dma_unmap_len(info, maplen)) {
			if (ACE_IS_TIGON_I(ap)) {
				/* NB: TIGON_1 is special, tx_ring is in io space */
				struct tx_desc __iomem *tx;
				tx = (__force struct tx_desc __iomem *) &ap->tx_ring[i];
				writel(0, &tx->addr.addrhi);
				writel(0, &tx->addr.addrlo);
				writel(0, &tx->flagsize);
			} else
				memset(ap->tx_ring + i, 0,
				       sizeof(struct tx_desc));
			pci_unmap_page(ap->pdev, dma_unmap_addr(info, mapping),
				       dma_unmap_len(info, maplen),
				       PCI_DMA_TODEVICE);
			dma_unmap_len_set(info, maplen, 0);
		}
		if (skb) {
			dev_kfree_skb(skb);
			info->skb = NULL;
		}
	}

	if (ap->jumbo) {
		cmd.evt = C_RESET_JUMBO_RNG;
		cmd.code = 0;
		cmd.idx = 0;
		ace_issue_cmd(regs, &cmd);
	}

	ace_unmask_irq(dev);
	local_irq_restore(flags);

	return 0;
}


static inline dma_addr_t
ace_map_tx_skb(struct ace_private *ap, struct sk_buff *skb,
	       struct sk_buff *tail, u32 idx)
{
	dma_addr_t mapping;
	struct tx_ring_info *info;

	mapping = pci_map_page(ap->pdev, virt_to_page(skb->data),
			       offset_in_page(skb->data),
			       skb->len, PCI_DMA_TODEVICE);

	info = ap->skb->tx_skbuff + idx;
	info->skb = tail;
	dma_unmap_addr_set(info, mapping, mapping);
	dma_unmap_len_set(info, maplen, skb->len);
	return mapping;
}


static inline void
ace_load_tx_bd(struct ace_private *ap, struct tx_desc *desc, u64 addr,
	       u32 flagsize, u32 vlan_tag)
{
#if !USE_TX_COAL_NOW
	flagsize &= ~BD_FLG_COAL_NOW;
#endif

	if (ACE_IS_TIGON_I(ap)) {
		struct tx_desc __iomem *io = (__force struct tx_desc __iomem *) desc;
		writel(addr >> 32, &io->addr.addrhi);
		writel(addr & 0xffffffff, &io->addr.addrlo);
		writel(flagsize, &io->flagsize);
		writel(vlan_tag, &io->vlanres);
	} else {
		desc->addr.addrhi = addr >> 32;
		desc->addr.addrlo = addr;
		desc->flagsize = flagsize;
		desc->vlanres = vlan_tag;
	}
}


static netdev_tx_t ace_start_xmit(struct sk_buff *skb,
				  struct net_device *dev)
{
	struct ace_private *ap = netdev_priv(dev);
	struct ace_regs __iomem *regs = ap->regs;
	struct tx_desc *desc;
	u32 idx, flagsize;
	unsigned long maxjiff = jiffies + 3*HZ;

restart:
	idx = ap->tx_prd;

	if (tx_ring_full(ap, ap->tx_ret_csm, idx))
		goto overflow;

	if (!skb_shinfo(skb)->nr_frags)	{
		dma_addr_t mapping;
		u32 vlan_tag = 0;

		mapping = ace_map_tx_skb(ap, skb, skb, idx);
		flagsize = (skb->len << 16) | (BD_FLG_END);
		if (skb->ip_summed == CHECKSUM_PARTIAL)
			flagsize |= BD_FLG_TCP_UDP_SUM;
		if (skb_vlan_tag_present(skb)) {
			flagsize |= BD_FLG_VLAN_TAG;
			vlan_tag = skb_vlan_tag_get(skb);
		}
		desc = ap->tx_ring + idx;
		idx = (idx + 1) % ACE_TX_RING_ENTRIES(ap);

		/* Look at ace_tx_int for explanations. */
		if (tx_ring_full(ap, ap->tx_ret_csm, idx))
			flagsize |= BD_FLG_COAL_NOW;

		ace_load_tx_bd(ap, desc, mapping, flagsize, vlan_tag);
	} else {
		dma_addr_t mapping;
		u32 vlan_tag = 0;
		int i, len = 0;

		mapping = ace_map_tx_skb(ap, skb, NULL, idx);
		flagsize = (skb_headlen(skb) << 16);
		if (skb->ip_summed == CHECKSUM_PARTIAL)
			flagsize |= BD_FLG_TCP_UDP_SUM;
		if (skb_vlan_tag_present(skb)) {
			flagsize |= BD_FLG_VLAN_TAG;
			vlan_tag = skb_vlan_tag_get(skb);
		}

		ace_load_tx_bd(ap, ap->tx_ring + idx, mapping, flagsize, vlan_tag);

		idx = (idx + 1) % ACE_TX_RING_ENTRIES(ap);

		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
			const skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
			struct tx_ring_info *info;

			len += skb_frag_size(frag);
			info = ap->skb->tx_skbuff + idx;
			desc = ap->tx_ring + idx;

			mapping = skb_frag_dma_map(&ap->pdev->dev, frag, 0,
						   skb_frag_size(frag),
						   DMA_TO_DEVICE);

			flagsize = skb_frag_size(frag) << 16;
			if (skb->ip_summed == CHECKSUM_PARTIAL)
				flagsize |= BD_FLG_TCP_UDP_SUM;
			idx = (idx + 1) % ACE_TX_RING_ENTRIES(ap);

			if (i == skb_shinfo(skb)->nr_frags - 1) {
				flagsize |= BD_FLG_END;
				if (tx_ring_full(ap, ap->tx_ret_csm, idx))
					flagsize |= BD_FLG_COAL_NOW;

				/*
				 * Only the last fragment frees
				 * the skb!
				 */
				info->skb = skb;
			} else {
				info->skb = NULL;
			}
			dma_unmap_addr_set(info, mapping, mapping);
			dma_unmap_len_set(info, maplen, skb_frag_size(frag));
			ace_load_tx_bd(ap, desc, mapping, flagsize, vlan_tag);
		}
	}

 	wmb();
 	ap->tx_prd = idx;
 	ace_set_txprd(regs, ap, idx);

	if (flagsize & BD_FLG_COAL_NOW) {
		netif_stop_queue(dev);

		/*
		 * A TX-descriptor producer (an IRQ) might have gotten
		 * between, making the ring free again. Since xmit is
		 * serialized, this is the only situation we have to
		 * re-test.
		 */
		if (!tx_ring_full(ap, ap->tx_ret_csm, idx))
			netif_wake_queue(dev);
	}

	return NETDEV_TX_OK;

overflow:
	/*
	 * This race condition is unavoidable with lock-free drivers.
	 * We wake up the queue _before_ tx_prd is advanced, so that we can
	 * enter hard_start_xmit too early, while tx ring still looks closed.
	 * This happens ~1-4 times per 100000 packets, so that we can allow
	 * to loop syncing to other CPU. Probably, we need an additional
	 * wmb() in ace_tx_intr as well.
	 *
	 * Note that this race is relieved by reserving one more entry
	 * in tx ring than it is necessary (see original non-SG driver).
	 * However, with SG we need to reserve 2*MAX_SKB_FRAGS+1, which
	 * is already overkill.
	 *
	 * Alternative is to return with 1 not throttling queue. In this
	 * case loop becomes longer, no more useful effects.
	 */
	if (time_before(jiffies, maxjiff)) {
		barrier();
		cpu_relax();
		goto restart;
	}

	/* The ring is stuck full. */
	printk(KERN_WARNING "%s: Transmit ring stuck full\n", dev->name);
	return NETDEV_TX_BUSY;
}


static int ace_change_mtu(struct net_device *dev, int new_mtu)
{
	struct ace_private *ap = netdev_priv(dev);
	struct ace_regs __iomem *regs = ap->regs;

	writel(new_mtu + ETH_HLEN + 4, &regs->IfMtu);
	dev->mtu = new_mtu;

	if (new_mtu > ACE_STD_MTU) {
		if (!(ap->jumbo)) {
			printk(KERN_INFO "%s: Enabling Jumbo frame "
			       "support\n", dev->name);
			ap->jumbo = 1;
			if (!test_and_set_bit(0, &ap->jumbo_refill_busy))
				ace_load_jumbo_rx_ring(dev, RX_JUMBO_SIZE);
			ace_set_rxtx_parms(dev, 1);
		}
	} else {
		while (test_and_set_bit(0, &ap->jumbo_refill_busy));
		ace_sync_irq(dev->irq);
		ace_set_rxtx_parms(dev, 0);
		if (ap->jumbo) {
			struct cmd cmd;

			cmd.evt = C_RESET_JUMBO_RNG;
			cmd.code = 0;
			cmd.idx = 0;
			ace_issue_cmd(regs, &cmd);
		}
	}

	return 0;
}

static int ace_get_link_ksettings(struct net_device *dev,
				  struct ethtool_link_ksettings *cmd)
{
	struct ace_private *ap = netdev_priv(dev);
	struct ace_regs __iomem *regs = ap->regs;
	u32 link;
	u32 supported;

	memset(cmd, 0, sizeof(struct ethtool_link_ksettings));

	supported = (SUPPORTED_10baseT_Half | SUPPORTED_10baseT_Full |
		     SUPPORTED_100baseT_Half | SUPPORTED_100baseT_Full |
		     SUPPORTED_1000baseT_Half | SUPPORTED_1000baseT_Full |
		     SUPPORTED_Autoneg | SUPPORTED_FIBRE);

	cmd->base.port = PORT_FIBRE;

	link = readl(&regs->GigLnkState);
	if (link & LNK_1000MB) {
		cmd->base.speed = SPEED_1000;
	} else {
		link = readl(&regs->FastLnkState);
		if (link & LNK_100MB)
			cmd->base.speed = SPEED_100;
		else if (link & LNK_10MB)
			cmd->base.speed = SPEED_10;
		else
			cmd->base.speed = 0;
	}
	if (link & LNK_FULL_DUPLEX)
		cmd->base.duplex = DUPLEX_FULL;
	else
		cmd->base.duplex = DUPLEX_HALF;

	if (link & LNK_NEGOTIATE)
		cmd->base.autoneg = AUTONEG_ENABLE;
	else
		cmd->base.autoneg = AUTONEG_DISABLE;

#if 0
	/*
	 * Current struct ethtool_cmd is insufficient
	 */
	ecmd->trace = readl(&regs->TuneTrace);

	ecmd->txcoal = readl(&regs->TuneTxCoalTicks);
	ecmd->rxcoal = readl(&regs->TuneRxCoalTicks);
#endif

	ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.supported,
						supported);

	return 0;
}

static int ace_set_link_ksettings(struct net_device *dev,
				  const struct ethtool_link_ksettings *cmd)
{
	struct ace_private *ap = netdev_priv(dev);
	struct ace_regs __iomem *regs = ap->regs;
	u32 link, speed;

	link = readl(&regs->GigLnkState);
	if (link & LNK_1000MB)
		speed = SPEED_1000;
	else {
		link = readl(&regs->FastLnkState);
		if (link & LNK_100MB)
			speed = SPEED_100;
		else if (link & LNK_10MB)
			speed = SPEED_10;
		else
			speed = SPEED_100;
	}

	link = LNK_ENABLE | LNK_1000MB | LNK_100MB | LNK_10MB |
		LNK_RX_FLOW_CTL_Y | LNK_NEG_FCTL;
	if (!ACE_IS_TIGON_I(ap))
		link |= LNK_TX_FLOW_CTL_Y;
	if (cmd->base.autoneg == AUTONEG_ENABLE)
		link |= LNK_NEGOTIATE;
	if (cmd->base.speed != speed) {
		link &= ~(LNK_1000MB | LNK_100MB | LNK_10MB);
		switch (cmd->base.speed) {
		case SPEED_1000:
			link |= LNK_1000MB;
			break;
		case SPEED_100:
			link |= LNK_100MB;
			break;
		case SPEED_10:
			link |= LNK_10MB;
			break;
		}
	}

	if (cmd->base.duplex == DUPLEX_FULL)
		link |= LNK_FULL_DUPLEX;

	if (link != ap->link) {
		struct cmd cmd;
		printk(KERN_INFO "%s: Renegotiating link state\n",
		       dev->name);

		ap->link = link;
		writel(link, &regs->TuneLink);
		if (!ACE_IS_TIGON_I(ap))
			writel(link, &regs->TuneFastLink);
		wmb();

		cmd.evt = C_LNK_NEGOTIATION;
		cmd.code = 0;
		cmd.idx = 0;
		ace_issue_cmd(regs, &cmd);
	}
	return 0;
}

static void ace_get_drvinfo(struct net_device *dev,
			    struct ethtool_drvinfo *info)
{
	struct ace_private *ap = netdev_priv(dev);

	strlcpy(info->driver, "acenic", sizeof(info->driver));
	snprintf(info->version, sizeof(info->version), "%i.%i.%i",
		 ap->firmware_major, ap->firmware_minor,
		 ap->firmware_fix);

	if (ap->pdev)
		strlcpy(info->bus_info, pci_name(ap->pdev),
			sizeof(info->bus_info));

}

/*
 * Set the hardware MAC address.
 */
static int ace_set_mac_addr(struct net_device *dev, void *p)
{
	struct ace_private *ap = netdev_priv(dev);
	struct ace_regs __iomem *regs = ap->regs;
	struct sockaddr *addr=p;
	u8 *da;
	struct cmd cmd;

	if(netif_running(dev))
		return -EBUSY;

	memcpy(dev->dev_addr, addr->sa_data,dev->addr_len);

	da = (u8 *)dev->dev_addr;

	writel(da[0] << 8 | da[1], &regs->MacAddrHi);
	writel((da[2] << 24) | (da[3] << 16) | (da[4] << 8) | da[5],
	       &regs->MacAddrLo);

	cmd.evt = C_SET_MAC_ADDR;
	cmd.code = 0;
	cmd.idx = 0;
	ace_issue_cmd(regs, &cmd);

	return 0;
}


static void ace_set_multicast_list(struct net_device *dev)
{
	struct ace_private *ap = netdev_priv(dev);
	struct ace_regs __iomem *regs = ap->regs;
	struct cmd cmd;

	if ((dev->flags & IFF_ALLMULTI) && !(ap->mcast_all)) {
		cmd.evt = C_SET_MULTICAST_MODE;
		cmd.code = C_C_MCAST_ENABLE;
		cmd.idx = 0;
		ace_issue_cmd(regs, &cmd);
		ap->mcast_all = 1;
	} else if (ap->mcast_all) {
		cmd.evt = C_SET_MULTICAST_MODE;
		cmd.code = C_C_MCAST_DISABLE;
		cmd.idx = 0;
		ace_issue_cmd(regs, &cmd);
		ap->mcast_all = 0;
	}

	if ((dev->flags & IFF_PROMISC) && !(ap->promisc)) {
		cmd.evt = C_SET_PROMISC_MODE;
		cmd.code = C_C_PROMISC_ENABLE;
		cmd.idx = 0;
		ace_issue_cmd(regs, &cmd);
		ap->promisc = 1;
	}else if (!(dev->flags & IFF_PROMISC) && (ap->promisc)) {
		cmd.evt = C_SET_PROMISC_MODE;
		cmd.code = C_C_PROMISC_DISABLE;
		cmd.idx = 0;
		ace_issue_cmd(regs, &cmd);
		ap->promisc = 0;
	}

	/*
	 * For the time being multicast relies on the upper layers
	 * filtering it properly. The Firmware does not allow one to
	 * set the entire multicast list at a time and keeping track of
	 * it here is going to be messy.
	 */
	if (!netdev_mc_empty(dev) && !ap->mcast_all) {
		cmd.evt = C_SET_MULTICAST_MODE;
		cmd.code = C_C_MCAST_ENABLE;
		cmd.idx = 0;
		ace_issue_cmd(regs, &cmd);
	}else if (!ap->mcast_all) {
		cmd.evt = C_SET_MULTICAST_MODE;
		cmd.code = C_C_MCAST_DISABLE;
		cmd.idx = 0;
		ace_issue_cmd(regs, &cmd);
	}
}


static struct net_device_stats *ace_get_stats(struct net_device *dev)
{
	struct ace_private *ap = netdev_priv(dev);
	struct ace_mac_stats __iomem *mac_stats =
		(struct ace_mac_stats __iomem *)ap->regs->Stats;

	dev->stats.rx_missed_errors = readl(&mac_stats->drop_space);
	dev->stats.multicast = readl(&mac_stats->kept_mc);
	dev->stats.collisions = readl(&mac_stats->coll);

	return &dev->stats;
}


static void ace_copy(struct ace_regs __iomem *regs, const __be32 *src,
		     u32 dest, int size)
{
	void __iomem *tdest;
	short tsize, i;

	if (size <= 0)
		return;

	while (size > 0) {
		tsize = min_t(u32, ((~dest & (ACE_WINDOW_SIZE - 1)) + 1),
			    min_t(u32, size, ACE_WINDOW_SIZE));
		tdest = (void __iomem *) &regs->Window +
			(dest & (ACE_WINDOW_SIZE - 1));
		writel(dest & ~(ACE_WINDOW_SIZE - 1), &regs->WinBase);
		for (i = 0; i < (tsize / 4); i++) {
			/* Firmware is big-endian */
			writel(be32_to_cpup(src), tdest);
			src++;
			tdest += 4;
			dest += 4;
			size -= 4;
		}
	}
}


static void ace_clear(struct ace_regs __iomem *regs, u32 dest, int size)
{
	void __iomem *tdest;
	short tsize = 0, i;

	if (size <= 0)
		return;

	while (size > 0) {
		tsize = min_t(u32, ((~dest & (ACE_WINDOW_SIZE - 1)) + 1),
				min_t(u32, size, ACE_WINDOW_SIZE));
		tdest = (void __iomem *) &regs->Window +
			(dest & (ACE_WINDOW_SIZE - 1));
		writel(dest & ~(ACE_WINDOW_SIZE - 1), &regs->WinBase);

		for (i = 0; i < (tsize / 4); i++) {
			writel(0, tdest + i*4);
		}

		dest += tsize;
		size -= tsize;
	}
}


/*
 * Download the firmware into the SRAM on the NIC
 *
 * This operation requires the NIC to be halted and is performed with
 * interrupts disabled and with the spinlock hold.
 */
static int ace_load_firmware(struct net_device *dev)
{
	const struct firmware *fw;
	const char *fw_name = "acenic/tg2.bin";
	struct ace_private *ap = netdev_priv(dev);
	struct ace_regs __iomem *regs = ap->regs;
	const __be32 *fw_data;
	u32 load_addr;
	int ret;

	if (!(readl(&regs->CpuCtrl) & CPU_HALTED)) {
		printk(KERN_ERR "%s: trying to download firmware while the "
		       "CPU is running!\n", ap->name);
		return -EFAULT;
	}

	if (ACE_IS_TIGON_I(ap))
		fw_name = "acenic/tg1.bin";

	ret = request_firmware(&fw, fw_name, &ap->pdev->dev);
	if (ret) {
		printk(KERN_ERR "%s: Failed to load firmware \"%s\"\n",
		       ap->name, fw_name);
		return ret;
	}

	fw_data = (void *)fw->data;

	/* Firmware blob starts with version numbers, followed by
	   load and start address. Remainder is the blob to be loaded
	   contiguously from load address. We don't bother to represent
	   the BSS/SBSS sections any more, since we were clearing the
	   whole thing anyway. */
	ap->firmware_major = fw->data[0];
	ap->firmware_minor = fw->data[1];
	ap->firmware_fix = fw->data[2];

	ap->firmware_start = be32_to_cpu(fw_data[1]);
	if (ap->firmware_start < 0x4000 || ap->firmware_start >= 0x80000) {
		printk(KERN_ERR "%s: bogus load address %08x in \"%s\"\n",
		       ap->name, ap->firmware_start, fw_name);
		ret = -EINVAL;
		goto out;
	}

	load_addr = be32_to_cpu(fw_data[2]);
	if (load_addr < 0x4000 || load_addr >= 0x80000) {
		printk(KERN_ERR "%s: bogus load address %08x in \"%s\"\n",
		       ap->name, load_addr, fw_name);
		ret = -EINVAL;
		goto out;
	}

	/*
	 * Do not try to clear more than 512KiB or we end up seeing
	 * funny things on NICs with only 512KiB SRAM
	 */
	ace_clear(regs, 0x2000, 0x80000-0x2000);
	ace_copy(regs, &fw_data[3], load_addr, fw->size-12);
 out:
	release_firmware(fw);
	return ret;
}


/*
 * The eeprom on the AceNIC is an Atmel i2c EEPROM.
 *
 * Accessing the EEPROM is `interesting' to say the least - don't read
 * this code right after dinner.
 *
 * This is all about black magic and bit-banging the device .... I
 * wonder in what hospital they have put the guy who designed the i2c
 * specs.
 *
 * Oh yes, this is only the beginning!
 *
 * Thanks to Stevarino Webinski for helping tracking down the bugs in the
 * code i2c readout code by beta testing all my hacks.
 */
static void eeprom_start(struct ace_regs __iomem *regs)
{
	u32 local;

	readl(&regs->LocalCtrl);
	udelay(ACE_SHORT_DELAY);
	local = readl(&regs->LocalCtrl);
	local |= EEPROM_DATA_OUT | EEPROM_WRITE_ENABLE;
	writel(local, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();
	udelay(ACE_SHORT_DELAY);
	local |= EEPROM_CLK_OUT;
	writel(local, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();
	udelay(ACE_SHORT_DELAY);
	local &= ~EEPROM_DATA_OUT;
	writel(local, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();
	udelay(ACE_SHORT_DELAY);
	local &= ~EEPROM_CLK_OUT;
	writel(local, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();
}


static void eeprom_prep(struct ace_regs __iomem *regs, u8 magic)
{
	short i;
	u32 local;

	udelay(ACE_SHORT_DELAY);
	local = readl(&regs->LocalCtrl);
	local &= ~EEPROM_DATA_OUT;
	local |= EEPROM_WRITE_ENABLE;
	writel(local, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();

	for (i = 0; i < 8; i++, magic <<= 1) {
		udelay(ACE_SHORT_DELAY);
		if (magic & 0x80)
			local |= EEPROM_DATA_OUT;
		else
			local &= ~EEPROM_DATA_OUT;
		writel(local, &regs->LocalCtrl);
		readl(&regs->LocalCtrl);
		mb();

		udelay(ACE_SHORT_DELAY);
		local |= EEPROM_CLK_OUT;
		writel(local, &regs->LocalCtrl);
		readl(&regs->LocalCtrl);
		mb();
		udelay(ACE_SHORT_DELAY);
		local &= ~(EEPROM_CLK_OUT | EEPROM_DATA_OUT);
		writel(local, &regs->LocalCtrl);
		readl(&regs->LocalCtrl);
		mb();
	}
}


static int eeprom_check_ack(struct ace_regs __iomem *regs)
{
	int state;
	u32 local;

	local = readl(&regs->LocalCtrl);
	local &= ~EEPROM_WRITE_ENABLE;
	writel(local, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();
	udelay(ACE_LONG_DELAY);
	local |= EEPROM_CLK_OUT;
	writel(local, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();
	udelay(ACE_SHORT_DELAY);
	/* sample data in middle of high clk */
	state = (readl(&regs->LocalCtrl) & EEPROM_DATA_IN) != 0;
	udelay(ACE_SHORT_DELAY);
	mb();
	writel(readl(&regs->LocalCtrl) & ~EEPROM_CLK_OUT, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();

	return state;
}


static void eeprom_stop(struct ace_regs __iomem *regs)
{
	u32 local;

	udelay(ACE_SHORT_DELAY);
	local = readl(&regs->LocalCtrl);
	local |= EEPROM_WRITE_ENABLE;
	writel(local, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();
	udelay(ACE_SHORT_DELAY);
	local &= ~EEPROM_DATA_OUT;
	writel(local, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();
	udelay(ACE_SHORT_DELAY);
	local |= EEPROM_CLK_OUT;
	writel(local, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();
	udelay(ACE_SHORT_DELAY);
	local |= EEPROM_DATA_OUT;
	writel(local, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();
	udelay(ACE_LONG_DELAY);
	local &= ~EEPROM_CLK_OUT;
	writel(local, &regs->LocalCtrl);
	mb();
}


/*
 * Read a whole byte from the EEPROM.
 */
static int read_eeprom_byte(struct net_device *dev, unsigned long offset)
{
	struct ace_private *ap = netdev_priv(dev);
	struct ace_regs __iomem *regs = ap->regs;
	unsigned long flags;
	u32 local;
	int result = 0;
	short i;

	/*
	 * Don't take interrupts on this CPU will bit banging
	 * the %#%#@$ I2C device
	 */
	local_irq_save(flags);

	eeprom_start(regs);

	eeprom_prep(regs, EEPROM_WRITE_SELECT);
	if (eeprom_check_ack(regs)) {
		local_irq_restore(flags);
		printk(KERN_ERR "%s: Unable to sync eeprom\n", ap->name);
		result = -EIO;
		goto eeprom_read_error;
	}

	eeprom_prep(regs, (offset >> 8) & 0xff);
	if (eeprom_check_ack(regs)) {
		local_irq_restore(flags);
		printk(KERN_ERR "%s: Unable to set address byte 0\n",
		       ap->name);
		result = -EIO;
		goto eeprom_read_error;
	}

	eeprom_prep(regs, offset & 0xff);
	if (eeprom_check_ack(regs)) {
		local_irq_restore(flags);
		printk(KERN_ERR "%s: Unable to set address byte 1\n",
		       ap->name);
		result = -EIO;
		goto eeprom_read_error;
	}

	eeprom_start(regs);
	eeprom_prep(regs, EEPROM_READ_SELECT);
	if (eeprom_check_ack(regs)) {
		local_irq_restore(flags);
		printk(KERN_ERR "%s: Unable to set READ_SELECT\n",
		       ap->name);
		result = -EIO;
		goto eeprom_read_error;
	}

	for (i = 0; i < 8; i++) {
		local = readl(&regs->LocalCtrl);
		local &= ~EEPROM_WRITE_ENABLE;
		writel(local, &regs->LocalCtrl);
		readl(&regs->LocalCtrl);
		udelay(ACE_LONG_DELAY);
		mb();
		local |= EEPROM_CLK_OUT;
		writel(local, &regs->LocalCtrl);
		readl(&regs->LocalCtrl);
		mb();
		udelay(ACE_SHORT_DELAY);
		/* sample data mid high clk */
		result = (result << 1) |
			((readl(&regs->LocalCtrl) & EEPROM_DATA_IN) != 0);
		udelay(ACE_SHORT_DELAY);
		mb();
		local = readl(&regs->LocalCtrl);
		local &= ~EEPROM_CLK_OUT;
		writel(local, &regs->LocalCtrl);
		readl(&regs->LocalCtrl);
		udelay(ACE_SHORT_DELAY);
		mb();
		if (i == 7) {
			local |= EEPROM_WRITE_ENABLE;
			writel(local, &regs->LocalCtrl);
			readl(&regs->LocalCtrl);
			mb();
			udelay(ACE_SHORT_DELAY);
		}
	}

	local |= EEPROM_DATA_OUT;
	writel(local, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();
	udelay(ACE_SHORT_DELAY);
	writel(readl(&regs->LocalCtrl) | EEPROM_CLK_OUT, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	udelay(ACE_LONG_DELAY);
	writel(readl(&regs->LocalCtrl) & ~EEPROM_CLK_OUT, &regs->LocalCtrl);
	readl(&regs->LocalCtrl);
	mb();
	udelay(ACE_SHORT_DELAY);
	eeprom_stop(regs);

	local_irq_restore(flags);
 out:
	return result;

 eeprom_read_error:
	printk(KERN_ERR "%s: Unable to read eeprom byte 0x%02lx\n",
	       ap->name, offset);
	goto out;
}

module_pci_driver(acenic_pci_driver);