		    READ_ONCE(ctl->desc->ctrl) & WCN36xx_DXE_CTRL_EOP) {
			dma_unmap_single(wcn->dev, ctl->desc->src_addr_l,
					 ctl->skb->len, DMA_TO_DEVICE);
			info = IEEE80211_SKB_CB(ctl->skb);
			if (!(info->flags & IEEE80211_TX_CTL_REQ_TX_STATUS)) {
				/* Keep frame until TX status comes */
				ieee80211_free_txskb(wcn->hw, ctl->skb);
			}
{
	struct wcn36xx *wcn = (struct wcn36xx *)dev;
	int int_src, int_reason;
	bool transmitted = false;

	wcn36xx_dxe_read_register(wcn, WCN36XX_DXE_INT_SRC_RAW_REG, &int_src);

	if (int_src & WCN36XX_INT_MASK_CHAN_TX_H) {
		wcn36xx_dxe_read_register(wcn,
					  WCN36XX_DXE_CH_STATUS_REG_ADDR_TX_H,
					  &int_reason);

		wcn36xx_dxe_write_register(wcn,
					   WCN36XX_DXE_0_INT_CLR,
					   WCN36XX_INT_MASK_CHAN_TX_H);

		if (int_reason & WCN36XX_CH_STAT_INT_ERR_MASK ) {
			wcn36xx_dxe_write_register(wcn,
						   WCN36XX_DXE_0_INT_ERR_CLR,
						   WCN36XX_INT_MASK_CHAN_TX_H);

			wcn36xx_err("DXE IRQ reported error: 0x%x in high TX channel\n",
					int_src);
		}

		if (int_reason & WCN36XX_CH_STAT_INT_DONE_MASK) {
			wcn36xx_dxe_write_register(wcn,
						   WCN36XX_DXE_0_INT_DONE_CLR,
						   WCN36XX_INT_MASK_CHAN_TX_H);
		}

		if (int_reason & WCN36XX_CH_STAT_INT_ED_MASK) {
			wcn36xx_dxe_write_register(wcn,
						   WCN36XX_DXE_0_INT_ED_CLR,
						   WCN36XX_INT_MASK_CHAN_TX_H);
		}

		wcn36xx_dbg(WCN36XX_DBG_DXE, "dxe tx ready high, reason %08x\n",
			    int_reason);

		if (int_reason & (WCN36XX_CH_STAT_INT_DONE_MASK |
				  WCN36XX_CH_STAT_INT_ED_MASK)) {
			reap_tx_dxes(wcn, &wcn->dxe_tx_h_ch);
			transmitted = true;
		}
	}
				  WCN36XX_CH_STAT_INT_ED_MASK)) {
			reap_tx_dxes(wcn, &wcn->dxe_tx_h_ch);
			transmitted = true;
		}
	if (int_src & WCN36XX_INT_MASK_CHAN_TX_L) {
		wcn36xx_dxe_read_register(wcn,
					  WCN36XX_DXE_CH_STATUS_REG_ADDR_TX_L,
					  &int_reason);

		wcn36xx_dxe_write_register(wcn,
					   WCN36XX_DXE_0_INT_CLR,
					   WCN36XX_INT_MASK_CHAN_TX_L);


		if (int_reason & WCN36XX_CH_STAT_INT_ERR_MASK ) {
			wcn36xx_dxe_write_register(wcn,
						   WCN36XX_DXE_0_INT_ERR_CLR,
						   WCN36XX_INT_MASK_CHAN_TX_L);

			wcn36xx_err("DXE IRQ reported error: 0x%x in low TX channel\n",
					int_src);
		}
				  WCN36XX_CH_STAT_INT_ED_MASK)) {
			reap_tx_dxes(wcn, &wcn->dxe_tx_l_ch);
			transmitted = true;
		}
	if (int_reason & WCN36XX_CH_STAT_INT_ERR_MASK) {
		wcn36xx_dxe_write_register(wcn,
					   WCN36XX_DXE_0_INT_ERR_CLR,
					   int_mask);

		wcn36xx_err("DXE IRQ reported error on RX channel\n");
	}

	if (int_reason & WCN36XX_CH_STAT_INT_DONE_MASK)
		wcn36xx_dxe_write_register(wcn,
					   WCN36XX_DXE_0_INT_DONE_CLR,
					   int_mask);

	if (int_reason & WCN36XX_CH_STAT_INT_ED_MASK)
		wcn36xx_dxe_write_register(wcn,
					   WCN36XX_DXE_0_INT_ED_CLR,
					   int_mask);

	if (!(int_reason & (WCN36XX_CH_STAT_INT_DONE_MASK |
			    WCN36XX_CH_STAT_INT_ED_MASK)))
		return 0;

	spin_lock(&ch->lock);

	ctl = ch->head_blk_ctl;
	dxe = ctl->desc;

	while (!(READ_ONCE(dxe->ctrl) & WCN36xx_DXE_CTRL_VLD)) {
		skb = ctl->skb;
		dma_addr = dxe->dst_addr_l;
		ret = wcn36xx_dxe_fill_skb(wcn->dev, ctl, GFP_ATOMIC);
		if (0 == ret) {
			/* new skb allocation ok. Use the new one and queue
			 * the old one to network system.
			 */
			dma_unmap_single(wcn->dev, dma_addr, WCN36XX_PKT_SIZE,
					DMA_FROM_DEVICE);
			wcn36xx_rx_skb(wcn, skb);
		} /* else keep old skb not submitted and use it for rx DMA */

		dxe->ctrl = ctrl;
		ctl = ctl->next;
		dxe = ctl->desc;
	}
	wcn36xx_dxe_write_register(wcn, WCN36XX_DXE_ENCH_ADDR, en_mask);

	ch->head_blk_ctl = ctl;

	spin_unlock(&ch->lock);

	return 0;
}
		if (0 == ret) {
			/* new skb allocation ok. Use the new one and queue
			 * the old one to network system.
			 */
			dma_unmap_single(wcn->dev, dma_addr, WCN36XX_PKT_SIZE,
					DMA_FROM_DEVICE);
			wcn36xx_rx_skb(wcn, skb);
		} /* else keep old skb not submitted and use it for rx DMA */

		dxe->ctrl = ctrl;
		ctl = ctl->next;
		dxe = ctl->desc;
	}
	wcn36xx_dxe_write_register(wcn, WCN36XX_DXE_ENCH_ADDR, en_mask);

	ch->head_blk_ctl = ctl;

	spin_unlock(&ch->lock);

	return 0;
}

void wcn36xx_dxe_rx_frame(struct wcn36xx *wcn)
{
	int int_src;

	wcn36xx_dxe_read_register(wcn, WCN36XX_DXE_INT_SRC_RAW_REG, &int_src);

	/* RX_LOW_PRI */
	if (int_src & WCN36XX_DXE_INT_CH1_MASK)
		wcn36xx_rx_handle_packets(wcn, &wcn->dxe_rx_l_ch,
					  WCN36XX_DXE_CTRL_RX_L,
					  WCN36XX_DXE_INT_CH1_MASK,
					  WCN36XX_INT_MASK_CHAN_RX_L,
					  WCN36XX_DXE_CH_STATUS_REG_ADDR_RX_L);

	/* RX_HIGH_PRI */
	if (int_src & WCN36XX_DXE_INT_CH3_MASK)
		wcn36xx_rx_handle_packets(wcn, &wcn->dxe_rx_h_ch,
					  WCN36XX_DXE_CTRL_RX_H,
					  WCN36XX_DXE_INT_CH3_MASK,
					  WCN36XX_INT_MASK_CHAN_RX_H,
					  WCN36XX_DXE_CH_STATUS_REG_ADDR_RX_H);

	if (!int_src)
		wcn36xx_warn("No DXE interrupt pending\n");
}

int wcn36xx_dxe_allocate_mem_pools(struct wcn36xx *wcn)
{
	size_t s;
	void *cpu_addr;

	/* Allocate BD headers for MGMT frames */

	/* Where this come from ask QC */
	wcn->mgmt_mem_pool.chunk_size =	WCN36XX_BD_CHUNK_SIZE +
		16 - (WCN36XX_BD_CHUNK_SIZE % 8);

	s = wcn->mgmt_mem_pool.chunk_size * WCN36XX_DXE_CH_DESC_NUMB_TX_H;
	cpu_addr = dma_alloc_coherent(wcn->dev, s,
				      &wcn->mgmt_mem_pool.phy_addr,
				      GFP_KERNEL);
	if (!cpu_addr)
		goto out_err;

	wcn->mgmt_mem_pool.virt_addr = cpu_addr;

	/* Allocate BD headers for DATA frames */

	/* Where this come from ask QC */
	wcn->data_mem_pool.chunk_size = WCN36XX_BD_CHUNK_SIZE +
		16 - (WCN36XX_BD_CHUNK_SIZE % 8);

	s = wcn->data_mem_pool.chunk_size * WCN36XX_DXE_CH_DESC_NUMB_TX_L;
	cpu_addr = dma_alloc_coherent(wcn->dev, s,
				      &wcn->data_mem_pool.phy_addr,
				      GFP_KERNEL);
	if (!cpu_addr)
		goto out_err;

	wcn->data_mem_pool.virt_addr = cpu_addr;

	return 0;

out_err:
	wcn36xx_dxe_free_mem_pools(wcn);
	wcn36xx_err("Failed to allocate BD mempool\n");
	return -ENOMEM;
}

void wcn36xx_dxe_free_mem_pools(struct wcn36xx *wcn)
{
	if (wcn->mgmt_mem_pool.virt_addr)
		dma_free_coherent(wcn->dev, wcn->mgmt_mem_pool.chunk_size *
				  WCN36XX_DXE_CH_DESC_NUMB_TX_H,
				  wcn->mgmt_mem_pool.virt_addr,
				  wcn->mgmt_mem_pool.phy_addr);

	if (wcn->data_mem_pool.virt_addr) {
		dma_free_coherent(wcn->dev, wcn->data_mem_pool.chunk_size *
				  WCN36XX_DXE_CH_DESC_NUMB_TX_L,
				  wcn->data_mem_pool.virt_addr,
				  wcn->data_mem_pool.phy_addr);
	}
}

int wcn36xx_dxe_tx_frame(struct wcn36xx *wcn,
			 struct wcn36xx_vif *vif_priv,
			 struct wcn36xx_tx_bd *bd,
			 struct sk_buff *skb,
			 bool is_low)
{
	struct wcn36xx_dxe_desc *desc_bd, *desc_skb;
	struct wcn36xx_dxe_ctl *ctl_bd, *ctl_skb;
	struct wcn36xx_dxe_ch *ch = NULL;
	unsigned long flags;
	int ret;

	ch = is_low ? &wcn->dxe_tx_l_ch : &wcn->dxe_tx_h_ch;

	spin_lock_irqsave(&ch->lock, flags);
	ctl_bd = ch->head_blk_ctl;
	ctl_skb = ctl_bd->next;

	/*
	 * If skb is not null that means that we reached the tail of the ring
	 * hence ring is full. Stop queues to let mac80211 back off until ring
	 * has an empty slot again.
	 */
	if (NULL != ctl_skb->skb) {
		ieee80211_stop_queues(wcn->hw);
		wcn->queues_stopped = true;
		spin_unlock_irqrestore(&ch->lock, flags);
		return -EBUSY;
	}

	if (unlikely(ctl_skb->bd_cpu_addr)) {
		wcn36xx_err("bd_cpu_addr cannot be NULL for skb DXE\n");
		ret = -EINVAL;
		goto unlock;
	}

	desc_bd = ctl_bd->desc;
	desc_skb = ctl_skb->desc;

	ctl_bd->skb = NULL;

	/* write buffer descriptor */
	memcpy(ctl_bd->bd_cpu_addr, bd, sizeof(*bd));

	/* Set source address of the BD we send */
	desc_bd->src_addr_l = ctl_bd->bd_phy_addr;
	desc_bd->dst_addr_l = ch->dxe_wq;
	desc_bd->fr_len = sizeof(struct wcn36xx_tx_bd);

	wcn36xx_dbg(WCN36XX_DBG_DXE, "DXE TX\n");

	wcn36xx_dbg_dump(WCN36XX_DBG_DXE_DUMP, "DESC1 >>> ",
			 (char *)desc_bd, sizeof(*desc_bd));
	wcn36xx_dbg_dump(WCN36XX_DBG_DXE_DUMP,
			 "BD   >>> ", (char *)ctl_bd->bd_cpu_addr,
			 sizeof(struct wcn36xx_tx_bd));

	desc_skb->src_addr_l = dma_map_single(wcn->dev,
					      skb->data,
					      skb->len,
					      DMA_TO_DEVICE);
	if (dma_mapping_error(wcn->dev, desc_skb->src_addr_l)) {
		dev_err(wcn->dev, "unable to DMA map src_addr_l\n");
		ret = -ENOMEM;
		goto unlock;
	}

	ctl_skb->skb = skb;
	desc_skb->dst_addr_l = ch->dxe_wq;
	desc_skb->fr_len = ctl_skb->skb->len;

	wcn36xx_dbg_dump(WCN36XX_DBG_DXE_DUMP, "DESC2 >>> ",
			 (char *)desc_skb, sizeof(*desc_skb));
	wcn36xx_dbg_dump(WCN36XX_DBG_DXE_DUMP, "SKB   >>> ",
			 (char *)ctl_skb->skb->data, ctl_skb->skb->len);

	/* Move the head of the ring to the next empty descriptor */
	ch->head_blk_ctl = ctl_skb->next;

	/* Commit all previous writes and set descriptors to VALID */
	wmb();
	desc_skb->ctrl = ch->ctrl_skb;
	wmb();
	desc_bd->ctrl = ch->ctrl_bd;

	/*
	 * When connected and trying to send data frame chip can be in sleep
	 * mode and writing to the register will not wake up the chip. Instead
	 * notify chip about new frame through SMSM bus.
	 */
	if (is_low &&  vif_priv->pw_state == WCN36XX_BMPS) {
		qcom_smem_state_update_bits(wcn->tx_rings_empty_state,
					    WCN36XX_SMSM_WLAN_TX_ENABLE,
					    WCN36XX_SMSM_WLAN_TX_ENABLE);
	} else {
		/* indicate End Of Packet and generate interrupt on descriptor
		 * done.
		 */
		wcn36xx_dxe_write_register(wcn,
			ch->reg_ctrl, ch->def_ctrl);
	}