		if (lp->tx_skbuff[tx_index]) {
			pci_unmap_single(lp->pci_dev, lp->tx_dma_addr[tx_index],
				  	lp->tx_skbuff[tx_index]->len,
					PCI_DMA_TODEVICE);
			dev_kfree_skb_irq (lp->tx_skbuff[tx_index]);
			lp->tx_skbuff[tx_index] = NULL;
			lp->tx_dma_addr[tx_index] = 0;
		}
		lp->tx_complete_idx++;
		/*COAL update tx coalescing parameters */
		lp->coal_conf.tx_packets++;
		lp->coal_conf.tx_bytes +=
			le16_to_cpu(lp->tx_ring[tx_index].buff_count);

		if (netif_queue_stopped(dev) &&
			lp->tx_complete_idx > lp->tx_idx - NUM_TX_BUFFERS +2){
			/* The ring is no longer full, clear tbusy. */
			/* lp->tx_full = 0; */
			netif_wake_queue (dev);
		}
	}
	return 0;
}

/* This function handles the driver receive operation in polling mode */
static int amd8111e_rx_poll(struct napi_struct *napi, int budget)
{
	struct amd8111e_priv *lp = container_of(napi, struct amd8111e_priv, napi);
	struct net_device *dev = lp->amd8111e_net_dev;
	int rx_index = lp->rx_idx & RX_RING_DR_MOD_MASK;
	void __iomem *mmio = lp->mmio;
	struct sk_buff *skb,*new_skb;
	int min_pkt_len, status;
	int num_rx_pkt = 0;
	short pkt_len;
#if AMD8111E_VLAN_TAG_USED
	short vtag;
#endif

	while (num_rx_pkt < budget) {
		status = le16_to_cpu(lp->rx_ring[rx_index].rx_flags);
		if (status & OWN_BIT)
			break;

		/* There is a tricky error noted by John Murphy,
		 * <murf@perftech.com> to Russ Nelson: Even with
		 * full-sized * buffers it's possible for a
		 * jabber packet to use two buffers, with only
		 * the last correctly noting the error.
		 */
		if (status & ERR_BIT) {
			/* resetting flags */
			lp->rx_ring[rx_index].rx_flags &= RESET_RX_FLAGS;
			goto err_next_pkt;
		}
		/* check for STP and ENP */
		if (!((status & STP_BIT) && (status & ENP_BIT))){
			/* resetting flags */
			lp->rx_ring[rx_index].rx_flags &= RESET_RX_FLAGS;
			goto err_next_pkt;
		}
		pkt_len = le16_to_cpu(lp->rx_ring[rx_index].msg_count) - 4;

#if AMD8111E_VLAN_TAG_USED
		vtag = status & TT_MASK;
		/* MAC will strip vlan tag */
		if (vtag != 0)
			min_pkt_len = MIN_PKT_LEN - 4;
			else
#endif
			min_pkt_len = MIN_PKT_LEN;

		if (pkt_len < min_pkt_len) {
			lp->rx_ring[rx_index].rx_flags &= RESET_RX_FLAGS;
			lp->drv_rx_errors++;
			goto err_next_pkt;
		}
		new_skb = netdev_alloc_skb(dev, lp->rx_buff_len);
		if (!new_skb) {
			/* if allocation fail,
			 * ignore that pkt and go to next one
			 */
			lp->rx_ring[rx_index].rx_flags &= RESET_RX_FLAGS;
			lp->drv_rx_errors++;
			goto err_next_pkt;
		}

		skb_reserve(new_skb, 2);
		skb = lp->rx_skbuff[rx_index];
		pci_unmap_single(lp->pci_dev,lp->rx_dma_addr[rx_index],
				 lp->rx_buff_len-2, PCI_DMA_FROMDEVICE);
		skb_put(skb, pkt_len);
		lp->rx_skbuff[rx_index] = new_skb;
		lp->rx_dma_addr[rx_index] = pci_map_single(lp->pci_dev,
							   new_skb->data,
							   lp->rx_buff_len-2,
							   PCI_DMA_FROMDEVICE);

		skb->protocol = eth_type_trans(skb, dev);

#if AMD8111E_VLAN_TAG_USED
		if (vtag == TT_VLAN_TAGGED){
			u16 vlan_tag = le16_to_cpu(lp->rx_ring[rx_index].tag_ctrl_info);
			__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vlan_tag);
		}
#endif
		napi_gro_receive(napi, skb);
		/* COAL update rx coalescing parameters */
		lp->coal_conf.rx_packets++;
		lp->coal_conf.rx_bytes += pkt_len;
		num_rx_pkt++;

err_next_pkt:
		lp->rx_ring[rx_index].buff_phy_addr
			= cpu_to_le32(lp->rx_dma_addr[rx_index]);
		lp->rx_ring[rx_index].buff_count =
			cpu_to_le16(lp->rx_buff_len-2);
		wmb();
		lp->rx_ring[rx_index].rx_flags |= cpu_to_le16(OWN_BIT);
		rx_index = (++lp->rx_idx) & RX_RING_DR_MOD_MASK;
	}

	if (num_rx_pkt < budget && napi_complete_done(napi, num_rx_pkt)) {
		unsigned long flags;

		/* Receive descriptor is empty now */
		spin_lock_irqsave(&lp->lock, flags);
		writel(VAL0|RINTEN0, mmio + INTEN0);
		writel(VAL2 | RDMD0, mmio + CMD0);
		spin_unlock_irqrestore(&lp->lock, flags);
	}

	return num_rx_pkt;
}

/* This function will indicate the link status to the kernel. */
static int amd8111e_link_change(struct net_device *dev)
{
	struct amd8111e_priv *lp = netdev_priv(dev);
	int status0,speed;

	/* read the link change */
     	status0 = readl(lp->mmio + STAT0);

	if(status0 & LINK_STATS){
		if(status0 & AUTONEG_COMPLETE)
			lp->link_config.autoneg = AUTONEG_ENABLE;
		else
			lp->link_config.autoneg = AUTONEG_DISABLE;

		if(status0 & FULL_DPLX)
			lp->link_config.duplex = DUPLEX_FULL;
		else
			lp->link_config.duplex = DUPLEX_HALF;
		speed = (status0 & SPEED_MASK) >> 7;
		if(speed == PHY_SPEED_10)
			lp->link_config.speed = SPEED_10;
		else if(speed == PHY_SPEED_100)
			lp->link_config.speed = SPEED_100;

		netdev_info(dev, "Link is Up. Speed is %s Mbps %s Duplex\n",
			    (lp->link_config.speed == SPEED_100) ?
							"100" : "10",
			    (lp->link_config.duplex == DUPLEX_FULL) ?
							"Full" : "Half");

		netif_carrier_on(dev);
	}
	else{
		lp->link_config.speed = SPEED_INVALID;
		lp->link_config.duplex = DUPLEX_INVALID;
		lp->link_config.autoneg = AUTONEG_INVALID;
		netdev_info(dev, "Link is Down.\n");
		netif_carrier_off(dev);
	}

	return 0;
}

/* This function reads the mib counters. */
static int amd8111e_read_mib(void __iomem *mmio, u8 MIB_COUNTER)
{
	unsigned int  status;
	unsigned  int data;
	unsigned int repeat = REPEAT_CNT;

	writew( MIB_RD_CMD | MIB_COUNTER, mmio + MIB_ADDR);
	do {
		status = readw(mmio + MIB_ADDR);
		udelay(2);	/* controller takes MAX 2 us to get mib data */
	}
	while (--repeat && (status & MIB_CMD_ACTIVE));

	data = readl(mmio + MIB_DATA);
	return data;
}

/* This function reads the mib registers and returns the hardware statistics.
 * It updates previous internal driver statistics with new values.
 */
static struct net_device_stats *amd8111e_get_stats(struct net_device *dev)
{
	struct amd8111e_priv *lp = netdev_priv(dev);
	void __iomem *mmio = lp->mmio;
	unsigned long flags;
	struct net_device_stats *new_stats = &dev->stats;

	if (!lp->opened)
		return new_stats;
	spin_lock_irqsave (&lp->lock, flags);

	/* stats.rx_packets */
	new_stats->rx_packets = amd8111e_read_mib(mmio, rcv_broadcast_pkts)+
				amd8111e_read_mib(mmio, rcv_multicast_pkts)+
				amd8111e_read_mib(mmio, rcv_unicast_pkts);

	/* stats.tx_packets */
	new_stats->tx_packets = amd8111e_read_mib(mmio, xmt_packets);

	/*stats.rx_bytes */
	new_stats->rx_bytes = amd8111e_read_mib(mmio, rcv_octets);

	/* stats.tx_bytes */
	new_stats->tx_bytes = amd8111e_read_mib(mmio, xmt_octets);

	/* stats.rx_errors */
	/* hw errors + errors driver reported */
	new_stats->rx_errors = amd8111e_read_mib(mmio, rcv_undersize_pkts)+
				amd8111e_read_mib(mmio, rcv_fragments)+
				amd8111e_read_mib(mmio, rcv_jabbers)+
				amd8111e_read_mib(mmio, rcv_alignment_errors)+
				amd8111e_read_mib(mmio, rcv_fcs_errors)+
				amd8111e_read_mib(mmio, rcv_miss_pkts)+
				lp->drv_rx_errors;

	/* stats.tx_errors */
	new_stats->tx_errors = amd8111e_read_mib(mmio, xmt_underrun_pkts);

	/* stats.rx_dropped*/
	new_stats->rx_dropped = amd8111e_read_mib(mmio, rcv_miss_pkts);

	/* stats.tx_dropped*/
	new_stats->tx_dropped = amd8111e_read_mib(mmio,  xmt_underrun_pkts);

	/* stats.multicast*/
	new_stats->multicast = amd8111e_read_mib(mmio, rcv_multicast_pkts);

	/* stats.collisions*/
	new_stats->collisions = amd8111e_read_mib(mmio, xmt_collisions);

	/* stats.rx_length_errors*/
	new_stats->rx_length_errors =
		amd8111e_read_mib(mmio, rcv_undersize_pkts)+
		amd8111e_read_mib(mmio, rcv_oversize_pkts);

	/* stats.rx_over_errors*/
	new_stats->rx_over_errors = amd8111e_read_mib(mmio, rcv_miss_pkts);

	/* stats.rx_crc_errors*/
	new_stats->rx_crc_errors = amd8111e_read_mib(mmio, rcv_fcs_errors);

	/* stats.rx_frame_errors*/
	new_stats->rx_frame_errors =
		amd8111e_read_mib(mmio, rcv_alignment_errors);

	/* stats.rx_fifo_errors */
	new_stats->rx_fifo_errors = amd8111e_read_mib(mmio, rcv_miss_pkts);

	/* stats.rx_missed_errors */
	new_stats->rx_missed_errors = amd8111e_read_mib(mmio, rcv_miss_pkts);

	/* stats.tx_aborted_errors*/
	new_stats->tx_aborted_errors =
		amd8111e_read_mib(mmio, xmt_excessive_collision);

	/* stats.tx_carrier_errors*/
	new_stats->tx_carrier_errors =
		amd8111e_read_mib(mmio, xmt_loss_carrier);

	/* stats.tx_fifo_errors*/
	new_stats->tx_fifo_errors = amd8111e_read_mib(mmio, xmt_underrun_pkts);

	/* stats.tx_window_errors*/
	new_stats->tx_window_errors =
		amd8111e_read_mib(mmio, xmt_late_collision);

	/* Reset the mibs for collecting new statistics */
	/* writew(MIB_CLEAR, mmio + MIB_ADDR);*/

	spin_unlock_irqrestore (&lp->lock, flags);

	return new_stats;
}

/* This function recalculate the interrupt coalescing  mode on every interrupt
 * according to the datarate and the packet rate.
 */
static int amd8111e_calc_coalesce(struct net_device *dev)
{
	struct amd8111e_priv *lp = netdev_priv(dev);
	struct amd8111e_coalesce_conf *coal_conf = &lp->coal_conf;
	int tx_pkt_rate;
	int rx_pkt_rate;
	int tx_data_rate;
	int rx_data_rate;
	int rx_pkt_size;
	int tx_pkt_size;

	tx_pkt_rate = coal_conf->tx_packets - coal_conf->tx_prev_packets;
	coal_conf->tx_prev_packets =  coal_conf->tx_packets;

	tx_data_rate = coal_conf->tx_bytes - coal_conf->tx_prev_bytes;
	coal_conf->tx_prev_bytes =  coal_conf->tx_bytes;

	rx_pkt_rate = coal_conf->rx_packets - coal_conf->rx_prev_packets;
	coal_conf->rx_prev_packets =  coal_conf->rx_packets;

	rx_data_rate = coal_conf->rx_bytes - coal_conf->rx_prev_bytes;
	coal_conf->rx_prev_bytes =  coal_conf->rx_bytes;

	if(rx_pkt_rate < 800){
		if(coal_conf->rx_coal_type != NO_COALESCE){

			coal_conf->rx_timeout = 0x0;
			coal_conf->rx_event_count = 0;
			amd8111e_set_coalesce(dev,RX_INTR_COAL);
			coal_conf->rx_coal_type = NO_COALESCE;
		}
	}
	else{

		rx_pkt_size = rx_data_rate/rx_pkt_rate;
		if (rx_pkt_size < 128){
			if(coal_conf->rx_coal_type != NO_COALESCE){

				coal_conf->rx_timeout = 0;
				coal_conf->rx_event_count = 0;
				amd8111e_set_coalesce(dev,RX_INTR_COAL);
				coal_conf->rx_coal_type = NO_COALESCE;
			}