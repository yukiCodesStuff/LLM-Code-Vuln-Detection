	enum rate_info_bw bw;
};

/* Buffer descriptor rx_ch field is limited to 5-bit (4+1), a mapping is used
 * for 11A Channels.
 */
static const u8 ab_rx_ch_map[] = { 36, 40, 44, 48, 52, 56, 60, 64, 100, 104,
				   108, 112, 116, 120, 124, 128, 132, 136, 140,
				   149, 153, 157, 161, 165, 144 };

static const struct wcn36xx_rate wcn36xx_rate_table[] = {
	/* 11b rates */
	{  10, 0, RX_ENC_LEGACY, 0, RATE_INFO_BW_20 },
	{  20, 1, RX_ENC_LEGACY, 0, RATE_INFO_BW_20 },
	{ 4333, 9, RX_ENC_VHT, RX_ENC_FLAG_SHORT_GI, RATE_INFO_BW_80 },
};

static struct sk_buff *wcn36xx_unchain_msdu(struct sk_buff_head *amsdu)
{
	struct sk_buff *skb, *first;
	int total_len = 0;
	int space;

	first = __skb_dequeue(amsdu);

	skb_queue_walk(amsdu, skb)
		total_len += skb->len;

	space = total_len - skb_tailroom(first);
	if (space > 0 && pskb_expand_head(first, 0, space, GFP_ATOMIC) < 0) {
		__skb_queue_head(amsdu, first);
		return NULL;
	}

	/* Walk list again, copying contents into msdu_head */
	while ((skb = __skb_dequeue(amsdu))) {
		skb_copy_from_linear_data(skb, skb_put(first, skb->len),
					  skb->len);
		dev_kfree_skb_irq(skb);
	}

	return first;
}

static void __skb_queue_purge_irq(struct sk_buff_head *list)
{
	struct sk_buff *skb;

	while ((skb = __skb_dequeue(list)) != NULL)
		dev_kfree_skb_irq(skb);
}

int wcn36xx_rx_skb(struct wcn36xx *wcn, struct sk_buff *skb)
{
	struct ieee80211_rx_status status;
	const struct wcn36xx_rate *rate;
			 "BD   <<< ", (char *)bd,
			 sizeof(struct wcn36xx_rx_bd));

	if (bd->pdu.mpdu_data_off <= bd->pdu.mpdu_header_off ||
	    bd->pdu.mpdu_len < bd->pdu.mpdu_header_len)
		goto drop;

	if (bd->asf && !bd->esf) { /* chained A-MSDU chunks */
		/* Sanity check */
		if (bd->pdu.mpdu_data_off + bd->pdu.mpdu_len > WCN36XX_PKT_SIZE)
			goto drop;

		skb_put(skb, bd->pdu.mpdu_data_off + bd->pdu.mpdu_len);
		skb_pull(skb, bd->pdu.mpdu_data_off);

		/* Only set status for first chained BD (with mac header) */
		goto done;
	}

	if (bd->pdu.mpdu_header_off < sizeof(*bd) ||
	    bd->pdu.mpdu_header_off + bd->pdu.mpdu_len > WCN36XX_PKT_SIZE)
		goto drop;

	skb_put(skb, bd->pdu.mpdu_header_off + bd->pdu.mpdu_len);
	skb_pull(skb, bd->pdu.mpdu_header_off);

	hdr = (struct ieee80211_hdr *) skb->data;
	    ieee80211_is_probe_resp(hdr->frame_control))
		status.boottime_ns = ktime_get_boottime_ns();

	if (bd->scan_learn) {
		/* If packet originates from hardware scanning, extract the
		 * band/channel from bd descriptor.
		 */
		u8 hwch = (bd->reserved0 << 4) + bd->rx_ch;

		if (bd->rf_band != 1 && hwch <= sizeof(ab_rx_ch_map) && hwch >= 1) {
			status.band = NL80211_BAND_5GHZ;
			status.freq = ieee80211_channel_to_frequency(ab_rx_ch_map[hwch - 1],
								     status.band);
		} else {
			status.band = NL80211_BAND_2GHZ;
			status.freq = ieee80211_channel_to_frequency(hwch, status.band);
		}
	}

	memcpy(IEEE80211_SKB_RXCB(skb), &status, sizeof(status));

	if (ieee80211_is_beacon(hdr->frame_control)) {
		wcn36xx_dbg(WCN36XX_DBG_BEACON, "beacon skb %p len %d fc %04x sn %d\n",
				 (char *)skb->data, skb->len);
	}

done:
	/*  Chained AMSDU ? slow path */
	if (unlikely(bd->asf && !(bd->lsf && bd->esf))) {
		if (bd->esf && !skb_queue_empty(&wcn->amsdu)) {
			wcn36xx_err("Discarding non complete chain");
			__skb_queue_purge_irq(&wcn->amsdu);
		}

		__skb_queue_tail(&wcn->amsdu, skb);

		if (!bd->lsf)
			return 0; /* Not the last AMSDU, wait for more */

		skb = wcn36xx_unchain_msdu(&wcn->amsdu);
		if (!skb)
			goto drop;
	}

	ieee80211_rx_irqsafe(wcn->hw, skb);

	return 0;

drop: /* drop everything */
	wcn36xx_err("Drop frame! skb:%p len:%u hoff:%u doff:%u asf=%u esf=%u lsf=%u\n",
		    skb, bd->pdu.mpdu_len, bd->pdu.mpdu_header_off,
		    bd->pdu.mpdu_data_off, bd->asf, bd->esf, bd->lsf);

	dev_kfree_skb_irq(skb);
	__skb_queue_purge_irq(&wcn->amsdu);

	return -EINVAL;
}

static void wcn36xx_set_tx_pdu(struct wcn36xx_tx_bd *bd,
			       u32 mpdu_header_len,
		bd->pdu.mpdu_header_off;
	bd->pdu.mpdu_len = len;
	bd->pdu.tid = tid;
}

static inline struct wcn36xx_vif *get_vif_by_addr(struct wcn36xx *wcn,
						  u8 *addr)
		tid = ieee80211_get_tid(hdr);
		/* TID->QID is one-to-one mapping */
		bd->queue_id = tid;
		bd->pdu.bd_ssn = WCN36XX_TXBD_SSN_FILL_DPU_QOS;
	} else {
		bd->pdu.bd_ssn = WCN36XX_TXBD_SSN_FILL_DPU_NON_QOS;
	}

	if (info->flags & IEEE80211_TX_INTFL_DONT_ENCRYPT ||
	    (sta_priv && !sta_priv->is_data_encrypted)) {
	if (ieee80211_is_any_nullfunc(hdr->frame_control)) {
		/* Don't use a regular queue for null packet (no ampdu) */
		bd->queue_id = WCN36XX_TX_U_WQ_ID;
		bd->bd_rate = WCN36XX_BD_RATE_CTRL;
		if (ieee80211_is_qos_nullfunc(hdr->frame_control))
			bd->pdu.bd_ssn = WCN36XX_TXBD_SSN_FILL_HOST;
	}

	if (bcast) {
		bd->ub = 1;
		bd->queue_id = WCN36XX_TX_U_WQ_ID;
	*vif_priv = __vif_priv;

	bd->pdu.bd_ssn = WCN36XX_TXBD_SSN_FILL_DPU_NON_QOS;

	wcn36xx_set_tx_pdu(bd,
			   ieee80211_is_data_qos(hdr->frame_control) ?
			   sizeof(struct ieee80211_qos_hdr) :
			   sizeof(struct ieee80211_hdr_3addr),
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct wcn36xx_vif *vif_priv = NULL;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	bool is_low = ieee80211_is_data(hdr->frame_control);
	bool bcast = is_broadcast_ether_addr(hdr->addr1) ||
		is_multicast_ether_addr(hdr->addr1);
	bool ack_ind = (info->flags & IEEE80211_TX_CTL_REQ_TX_STATUS) &&
					!(info->flags & IEEE80211_TX_CTL_NO_ACK);
	struct wcn36xx_tx_bd bd;
	int ret;

	memset(&bd, 0, sizeof(bd));

	bd.dpu_rf = WCN36XX_BMU_WQ_TX;

	if (unlikely(ack_ind)) {
		wcn36xx_dbg(WCN36XX_DBG_DXE, "TX_ACK status requested\n");

		/* Only one at a time is supported by fw. Stop the TX queues
		 * until the ack status gets back.
		 */
		ieee80211_stop_queues(wcn->hw);

		/* Request ack indication from the firmware */
		bd.tx_comp = 1;
	}

	/* Data frames served first*/
	if (is_low)
	bd.tx_bd_sign = 0xbdbdbdbd;

	ret = wcn36xx_dxe_tx_frame(wcn, vif_priv, &bd, skb, is_low);
	if (unlikely(ret && ack_ind)) {
		/* If the skb has not been transmitted, resume TX queue */
		ieee80211_wake_queues(wcn->hw);
	}

	return ret;