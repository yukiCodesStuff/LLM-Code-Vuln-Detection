			dma_unmap_single(wcn->dev, ctl->desc->src_addr_l,
					 ctl->skb->len, DMA_TO_DEVICE);
			info = IEEE80211_SKB_CB(ctl->skb);
			if (!(info->flags & IEEE80211_TX_CTL_REQ_TX_STATUS)) {
				/* Keep frame until TX status comes */
				ieee80211_free_txskb(wcn->hw, ctl->skb);
			}

			if (wcn->queues_stopped) {
{
	struct wcn36xx *wcn = (struct wcn36xx *)dev;
	int int_src, int_reason;
	bool transmitted = false;

	wcn36xx_dxe_read_register(wcn, WCN36XX_DXE_INT_SRC_RAW_REG, &int_src);

	if (int_src & WCN36XX_INT_MASK_CHAN_TX_H) {
		if (int_reason & (WCN36XX_CH_STAT_INT_DONE_MASK |
				  WCN36XX_CH_STAT_INT_ED_MASK)) {
			reap_tx_dxes(wcn, &wcn->dxe_tx_h_ch);
			transmitted = true;
		}
	}

	if (int_src & WCN36XX_INT_MASK_CHAN_TX_L) {
					   WCN36XX_DXE_0_INT_CLR,
					   WCN36XX_INT_MASK_CHAN_TX_L);


		if (int_reason & WCN36XX_CH_STAT_INT_ERR_MASK ) {
			wcn36xx_dxe_write_register(wcn,
						   WCN36XX_DXE_0_INT_ERR_CLR,
						   WCN36XX_INT_MASK_CHAN_TX_L);
		if (int_reason & (WCN36XX_CH_STAT_INT_DONE_MASK |
				  WCN36XX_CH_STAT_INT_ED_MASK)) {
			reap_tx_dxes(wcn, &wcn->dxe_tx_l_ch);
			transmitted = true;
		}
	}

	spin_lock(&wcn->dxe_lock);
	if (wcn->tx_ack_skb && transmitted) {
		struct ieee80211_tx_info *info = IEEE80211_SKB_CB(wcn->tx_ack_skb);

		/* TX complete, no need to wait for 802.11 ack indication */
		if (info->flags & IEEE80211_TX_CTL_REQ_TX_STATUS &&
		    info->flags & IEEE80211_TX_CTL_NO_ACK) {
			info->flags |= IEEE80211_TX_STAT_NOACK_TRANSMITTED;
			del_timer(&wcn->tx_ack_timer);
			ieee80211_tx_status_irqsafe(wcn->hw, wcn->tx_ack_skb);
			wcn->tx_ack_skb = NULL;
			ieee80211_wake_queues(wcn->hw);
		}
	}
	spin_unlock(&wcn->dxe_lock);

	return IRQ_HANDLED;
}

	dxe = ctl->desc;

	while (!(READ_ONCE(dxe->ctrl) & WCN36xx_DXE_CTRL_VLD)) {
		skb = ctl->skb;
		dma_addr = dxe->dst_addr_l;
		ret = wcn36xx_dxe_fill_skb(wcn->dev, ctl, GFP_ATOMIC);
		if (0 == ret) {
			dma_unmap_single(wcn->dev, dma_addr, WCN36XX_PKT_SIZE,
					DMA_FROM_DEVICE);
			wcn36xx_rx_skb(wcn, skb);
		} /* else keep old skb not submitted and use it for rx DMA */

		dxe->ctrl = ctrl;
		ctl = ctl->next;
		dxe = ctl->desc;
	}
	wcn36xx_dxe_write_register(wcn, WCN36XX_DXE_ENCH_ADDR, en_mask);