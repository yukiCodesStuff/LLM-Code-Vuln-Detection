	enum rate_info_bw bw;
};

static const struct wcn36xx_rate wcn36xx_rate_table[] = {
	/* 11b rates */
	{  10, 0, RX_ENC_LEGACY, 0, RATE_INFO_BW_20 },
	{  20, 1, RX_ENC_LEGACY, 0, RATE_INFO_BW_20 },
	{ 4333, 9, RX_ENC_VHT, RX_ENC_FLAG_SHORT_GI, RATE_INFO_BW_80 },
};

int wcn36xx_rx_skb(struct wcn36xx *wcn, struct sk_buff *skb)
{
	struct ieee80211_rx_status status;
	const struct wcn36xx_rate *rate;
			 "BD   <<< ", (char *)bd,
			 sizeof(struct wcn36xx_rx_bd));

	skb_put(skb, bd->pdu.mpdu_header_off + bd->pdu.mpdu_len);
	skb_pull(skb, bd->pdu.mpdu_header_off);

	hdr = (struct ieee80211_hdr *) skb->data;
	    ieee80211_is_probe_resp(hdr->frame_control))
		status.boottime_ns = ktime_get_boottime_ns();

	memcpy(IEEE80211_SKB_RXCB(skb), &status, sizeof(status));

	if (ieee80211_is_beacon(hdr->frame_control)) {
		wcn36xx_dbg(WCN36XX_DBG_BEACON, "beacon skb %p len %d fc %04x sn %d\n",
				 (char *)skb->data, skb->len);
	}

	ieee80211_rx_irqsafe(wcn->hw, skb);

	return 0;
}

static void wcn36xx_set_tx_pdu(struct wcn36xx_tx_bd *bd,
			       u32 mpdu_header_len,
		bd->pdu.mpdu_header_off;
	bd->pdu.mpdu_len = len;
	bd->pdu.tid = tid;
	/* Use seq number generated by mac80211 */
	bd->pdu.bd_ssn = WCN36XX_TXBD_SSN_FILL_HOST;
}

static inline struct wcn36xx_vif *get_vif_by_addr(struct wcn36xx *wcn,
						  u8 *addr)
		tid = ieee80211_get_tid(hdr);
		/* TID->QID is one-to-one mapping */
		bd->queue_id = tid;
	}

	if (info->flags & IEEE80211_TX_INTFL_DONT_ENCRYPT ||
	    (sta_priv && !sta_priv->is_data_encrypted)) {
	if (ieee80211_is_any_nullfunc(hdr->frame_control)) {
		/* Don't use a regular queue for null packet (no ampdu) */
		bd->queue_id = WCN36XX_TX_U_WQ_ID;
	}

	if (bcast) {
		bd->ub = 1;
		bd->queue_id = WCN36XX_TX_U_WQ_ID;
	*vif_priv = __vif_priv;

	wcn36xx_set_tx_pdu(bd,
			   ieee80211_is_data_qos(hdr->frame_control) ?
			   sizeof(struct ieee80211_qos_hdr) :
			   sizeof(struct ieee80211_hdr_3addr),
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct wcn36xx_vif *vif_priv = NULL;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	unsigned long flags;
	bool is_low = ieee80211_is_data(hdr->frame_control);
	bool bcast = is_broadcast_ether_addr(hdr->addr1) ||
		is_multicast_ether_addr(hdr->addr1);
	struct wcn36xx_tx_bd bd;
	int ret;

	memset(&bd, 0, sizeof(bd));

	bd.dpu_rf = WCN36XX_BMU_WQ_TX;

	if (info->flags & IEEE80211_TX_CTL_REQ_TX_STATUS) {
		wcn36xx_dbg(WCN36XX_DBG_DXE, "TX_ACK status requested\n");

		spin_lock_irqsave(&wcn->dxe_lock, flags);
		if (wcn->tx_ack_skb) {
			spin_unlock_irqrestore(&wcn->dxe_lock, flags);
			wcn36xx_warn("tx_ack_skb already set\n");
			return -EINVAL;
		}

		wcn->tx_ack_skb = skb;
		spin_unlock_irqrestore(&wcn->dxe_lock, flags);

		/* Only one at a time is supported by fw. Stop the TX queues
		 * until the ack status gets back.
		 */
		ieee80211_stop_queues(wcn->hw);

		/* TX watchdog if no TX irq or ack indication received  */
		mod_timer(&wcn->tx_ack_timer, jiffies + HZ / 10);

		/* Request ack indication from the firmware */
		if (!(info->flags & IEEE80211_TX_CTL_NO_ACK))
			bd.tx_comp = 1;
	}

	/* Data frames served first*/
	if (is_low)
	bd.tx_bd_sign = 0xbdbdbdbd;

	ret = wcn36xx_dxe_tx_frame(wcn, vif_priv, &bd, skb, is_low);
	if (ret && (info->flags & IEEE80211_TX_CTL_REQ_TX_STATUS)) {
		/* If the skb has not been transmitted,
		 * don't keep a reference to it.
		 */
		spin_lock_irqsave(&wcn->dxe_lock, flags);
		wcn->tx_ack_skb = NULL;
		spin_unlock_irqrestore(&wcn->dxe_lock, flags);

		ieee80211_wake_queues(wcn->hw);
	}

	return ret;