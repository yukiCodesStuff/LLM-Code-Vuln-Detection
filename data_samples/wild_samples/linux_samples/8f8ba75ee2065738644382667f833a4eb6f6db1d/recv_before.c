{
	struct ath_buf *bf;
	struct sk_buff *skb = NULL, *requeue_skb, *hdr_skb;
	struct ieee80211_rx_status *rxs;
	struct ath_hw *ah = sc->sc_ah;
	struct ath_common *common = ath9k_hw_common(ah);
	struct ieee80211_hw *hw = sc->hw;
	struct ieee80211_hdr *hdr;
	int retval;
	bool decrypt_error = false;
	struct ath_rx_status rs;
	enum ath9k_rx_qtype qtype;
	bool edma = !!(ah->caps.hw_caps & ATH9K_HW_CAP_EDMA);
	int dma_type;
	u8 rx_status_len = ah->caps.rx_status_len;
	u64 tsf = 0;
	u32 tsf_lower = 0;
	unsigned long flags;

	if (edma)
		dma_type = DMA_BIDIRECTIONAL;
	else
		dma_type = DMA_FROM_DEVICE;

	qtype = hp ? ATH9K_RX_QUEUE_HP : ATH9K_RX_QUEUE_LP;
	spin_lock_bh(&sc->rx.rxbuflock);

	tsf = ath9k_hw_gettsf64(ah);
	tsf_lower = tsf & 0xffffffff;

	do {
		/* If handling rx interrupt and flush is in progress => exit */
		if (test_bit(SC_OP_RXFLUSH, &sc->sc_flags) && (flush == 0))
			break;

		memset(&rs, 0, sizeof(rs));
		if (edma)
			bf = ath_edma_get_next_rx_buf(sc, &rs, qtype);
		else
			bf = ath_get_next_rx_buf(sc, &rs);

		if (!bf)
			break;

		skb = bf->bf_mpdu;
		if (!skb)
			continue;

		/*
		 * Take frame header from the first fragment and RX status from
		 * the last one.
		 */
		if (sc->rx.frag)
			hdr_skb = sc->rx.frag;
		else
			hdr_skb = skb;

		hdr = (struct ieee80211_hdr *) (hdr_skb->data + rx_status_len);
		rxs = IEEE80211_SKB_RXCB(hdr_skb);
		if (ieee80211_is_beacon(hdr->frame_control)) {
			RX_STAT_INC(rx_beacons);
			if (!is_zero_ether_addr(common->curbssid) &&
			    ether_addr_equal(hdr->addr3, common->curbssid))
				rs.is_mybeacon = true;
			else
				rs.is_mybeacon = false;
		}
		else
			rs.is_mybeacon = false;

		sc->rx.num_pkts++;
		ath_debug_stat_rx(sc, &rs);

		/*
		 * If we're asked to flush receive queue, directly
		 * chain it back at the queue without processing it.
		 */
		if (test_bit(SC_OP_RXFLUSH, &sc->sc_flags)) {
			RX_STAT_INC(rx_drop_rxflush);
			goto requeue_drop_frag;
		}

		memset(rxs, 0, sizeof(struct ieee80211_rx_status));

		rxs->mactime = (tsf & ~0xffffffffULL) | rs.rs_tstamp;
		if (rs.rs_tstamp > tsf_lower &&
		    unlikely(rs.rs_tstamp - tsf_lower > 0x10000000))
			rxs->mactime -= 0x100000000ULL;

		if (rs.rs_tstamp < tsf_lower &&
		    unlikely(tsf_lower - rs.rs_tstamp > 0x10000000))
			rxs->mactime += 0x100000000ULL;

		retval = ath9k_rx_skb_preprocess(common, hw, hdr, &rs,
						 rxs, &decrypt_error);
		if (retval)
			goto requeue_drop_frag;

		if (rs.is_mybeacon) {
			sc->hw_busy_count = 0;
			ath_start_rx_poll(sc, 3);
		}
		/* Ensure we always have an skb to requeue once we are done
		 * processing the current buffer's skb */
		requeue_skb = ath_rxbuf_alloc(common, common->rx_bufsize, GFP_ATOMIC);

		/* If there is no memory we ignore the current RX'd frame,
		 * tell hardware it can give us a new frame using the old
		 * skb and put it at the tail of the sc->rx.rxbuf list for
		 * processing. */
		if (!requeue_skb) {
			RX_STAT_INC(rx_oom_err);
			goto requeue_drop_frag;
		}

		/* Unmap the frame */
		dma_unmap_single(sc->dev, bf->bf_buf_addr,
				 common->rx_bufsize,
				 dma_type);

		skb_put(skb, rs.rs_datalen + ah->caps.rx_status_len);
		if (ah->caps.rx_status_len)
			skb_pull(skb, ah->caps.rx_status_len);

		if (!rs.rs_more)
			ath9k_rx_skb_postprocess(common, hdr_skb, &rs,
						 rxs, decrypt_error);

		/* We will now give hardware our shiny new allocated skb */
		bf->bf_mpdu = requeue_skb;
		bf->bf_buf_addr = dma_map_single(sc->dev, requeue_skb->data,
						 common->rx_bufsize,
						 dma_type);
		if (unlikely(dma_mapping_error(sc->dev,
			  bf->bf_buf_addr))) {
			dev_kfree_skb_any(requeue_skb);
			bf->bf_mpdu = NULL;
			bf->bf_buf_addr = 0;
			ath_err(common, "dma_mapping_error() on RX\n");
			ieee80211_rx(hw, skb);
			break;
		}

		if (rs.rs_more) {
			RX_STAT_INC(rx_frags);
			/*
			 * rs_more indicates chained descriptors which can be
			 * used to link buffers together for a sort of
			 * scatter-gather operation.
			 */
			if (sc->rx.frag) {
				/* too many fragments - cannot handle frame */
				dev_kfree_skb_any(sc->rx.frag);
				dev_kfree_skb_any(skb);
				RX_STAT_INC(rx_too_many_frags_err);
				skb = NULL;
			}
			sc->rx.frag = skb;
			goto requeue;
		}

		if (sc->rx.frag) {
			int space = skb->len - skb_tailroom(hdr_skb);

			if (pskb_expand_head(hdr_skb, 0, space, GFP_ATOMIC) < 0) {
				dev_kfree_skb(skb);
				RX_STAT_INC(rx_oom_err);
				goto requeue_drop_frag;
			}

			sc->rx.frag = NULL;

			skb_copy_from_linear_data(skb, skb_put(hdr_skb, skb->len),
						  skb->len);
			dev_kfree_skb_any(skb);
			skb = hdr_skb;
		}


		if (ah->caps.hw_caps & ATH9K_HW_CAP_ANT_DIV_COMB) {

			/*
			 * change the default rx antenna if rx diversity
			 * chooses the other antenna 3 times in a row.
			 */
			if (sc->rx.defant != rs.rs_antenna) {
				if (++sc->rx.rxotherant >= 3)
					ath_setdefantenna(sc, rs.rs_antenna);
			} else {
				sc->rx.rxotherant = 0;
			}

		}

		if (rxs->flag & RX_FLAG_MMIC_STRIPPED)
			skb_trim(skb, skb->len - 8);

		spin_lock_irqsave(&sc->sc_pm_lock, flags);
		if ((sc->ps_flags & (PS_WAIT_FOR_BEACON |
				     PS_WAIT_FOR_CAB |
				     PS_WAIT_FOR_PSPOLL_DATA)) ||
		    ath9k_check_auto_sleep(sc))
			ath_rx_ps(sc, skb, rs.is_mybeacon);
		spin_unlock_irqrestore(&sc->sc_pm_lock, flags);

		if ((ah->caps.hw_caps & ATH9K_HW_CAP_ANT_DIV_COMB) && sc->ant_rx == 3)
			ath_ant_comb_scan(sc, &rs);

		ieee80211_rx(hw, skb);

requeue_drop_frag:
		if (sc->rx.frag) {
			dev_kfree_skb_any(sc->rx.frag);
			sc->rx.frag = NULL;
		}
requeue:
		if (edma) {
			list_add_tail(&bf->list, &sc->rx.rxbuf);
			ath_rx_edma_buf_link(sc, qtype);
		} else {
			list_move_tail(&bf->list, &sc->rx.rxbuf);
			ath_rx_buf_link(sc, bf);
			if (!flush)
				ath9k_hw_rxena(ah);
		}
	} while (1);

	spin_unlock_bh(&sc->rx.rxbuflock);

	if (!(ah->imask & ATH9K_INT_RXEOL)) {
		ah->imask |= (ATH9K_INT_RXEOL | ATH9K_INT_RXORN);
		ath9k_hw_set_interrupts(ah);
	}

	return 0;
}
{
	struct ath_buf *bf;
	struct sk_buff *skb = NULL, *requeue_skb, *hdr_skb;
	struct ieee80211_rx_status *rxs;
	struct ath_hw *ah = sc->sc_ah;
	struct ath_common *common = ath9k_hw_common(ah);
	struct ieee80211_hw *hw = sc->hw;
	struct ieee80211_hdr *hdr;
	int retval;
	bool decrypt_error = false;
	struct ath_rx_status rs;
	enum ath9k_rx_qtype qtype;
	bool edma = !!(ah->caps.hw_caps & ATH9K_HW_CAP_EDMA);
	int dma_type;
	u8 rx_status_len = ah->caps.rx_status_len;
	u64 tsf = 0;
	u32 tsf_lower = 0;
	unsigned long flags;

	if (edma)
		dma_type = DMA_BIDIRECTIONAL;
	else
		dma_type = DMA_FROM_DEVICE;

	qtype = hp ? ATH9K_RX_QUEUE_HP : ATH9K_RX_QUEUE_LP;
	spin_lock_bh(&sc->rx.rxbuflock);

	tsf = ath9k_hw_gettsf64(ah);
	tsf_lower = tsf & 0xffffffff;

	do {
		/* If handling rx interrupt and flush is in progress => exit */
		if (test_bit(SC_OP_RXFLUSH, &sc->sc_flags) && (flush == 0))
			break;

		memset(&rs, 0, sizeof(rs));
		if (edma)
			bf = ath_edma_get_next_rx_buf(sc, &rs, qtype);
		else
			bf = ath_get_next_rx_buf(sc, &rs);

		if (!bf)
			break;

		skb = bf->bf_mpdu;
		if (!skb)
			continue;

		/*
		 * Take frame header from the first fragment and RX status from
		 * the last one.
		 */
		if (sc->rx.frag)
			hdr_skb = sc->rx.frag;
		else
			hdr_skb = skb;

		hdr = (struct ieee80211_hdr *) (hdr_skb->data + rx_status_len);
		rxs = IEEE80211_SKB_RXCB(hdr_skb);
		if (ieee80211_is_beacon(hdr->frame_control)) {
			RX_STAT_INC(rx_beacons);
			if (!is_zero_ether_addr(common->curbssid) &&
			    ether_addr_equal(hdr->addr3, common->curbssid))
				rs.is_mybeacon = true;
			else
				rs.is_mybeacon = false;
		}
		else
			rs.is_mybeacon = false;

		sc->rx.num_pkts++;
		ath_debug_stat_rx(sc, &rs);

		/*
		 * If we're asked to flush receive queue, directly
		 * chain it back at the queue without processing it.
		 */
		if (test_bit(SC_OP_RXFLUSH, &sc->sc_flags)) {
			RX_STAT_INC(rx_drop_rxflush);
			goto requeue_drop_frag;
		}

		memset(rxs, 0, sizeof(struct ieee80211_rx_status));

		rxs->mactime = (tsf & ~0xffffffffULL) | rs.rs_tstamp;
		if (rs.rs_tstamp > tsf_lower &&
		    unlikely(rs.rs_tstamp - tsf_lower > 0x10000000))
			rxs->mactime -= 0x100000000ULL;

		if (rs.rs_tstamp < tsf_lower &&
		    unlikely(tsf_lower - rs.rs_tstamp > 0x10000000))
			rxs->mactime += 0x100000000ULL;

		retval = ath9k_rx_skb_preprocess(common, hw, hdr, &rs,
						 rxs, &decrypt_error);
		if (retval)
			goto requeue_drop_frag;

		if (rs.is_mybeacon) {
			sc->hw_busy_count = 0;
			ath_start_rx_poll(sc, 3);
		}
		/* Ensure we always have an skb to requeue once we are done
		 * processing the current buffer's skb */
		requeue_skb = ath_rxbuf_alloc(common, common->rx_bufsize, GFP_ATOMIC);

		/* If there is no memory we ignore the current RX'd frame,
		 * tell hardware it can give us a new frame using the old
		 * skb and put it at the tail of the sc->rx.rxbuf list for
		 * processing. */
		if (!requeue_skb) {
			RX_STAT_INC(rx_oom_err);
			goto requeue_drop_frag;
		}

		/* Unmap the frame */
		dma_unmap_single(sc->dev, bf->bf_buf_addr,
				 common->rx_bufsize,
				 dma_type);

		skb_put(skb, rs.rs_datalen + ah->caps.rx_status_len);
		if (ah->caps.rx_status_len)
			skb_pull(skb, ah->caps.rx_status_len);

		if (!rs.rs_more)
			ath9k_rx_skb_postprocess(common, hdr_skb, &rs,
						 rxs, decrypt_error);

		/* We will now give hardware our shiny new allocated skb */
		bf->bf_mpdu = requeue_skb;
		bf->bf_buf_addr = dma_map_single(sc->dev, requeue_skb->data,
						 common->rx_bufsize,
						 dma_type);
		if (unlikely(dma_mapping_error(sc->dev,
			  bf->bf_buf_addr))) {
			dev_kfree_skb_any(requeue_skb);
			bf->bf_mpdu = NULL;
			bf->bf_buf_addr = 0;
			ath_err(common, "dma_mapping_error() on RX\n");
			ieee80211_rx(hw, skb);
			break;
		}

		if (rs.rs_more) {
			RX_STAT_INC(rx_frags);
			/*
			 * rs_more indicates chained descriptors which can be
			 * used to link buffers together for a sort of
			 * scatter-gather operation.
			 */
			if (sc->rx.frag) {
				/* too many fragments - cannot handle frame */
				dev_kfree_skb_any(sc->rx.frag);
				dev_kfree_skb_any(skb);
				RX_STAT_INC(rx_too_many_frags_err);
				skb = NULL;
			}
			sc->rx.frag = skb;
			goto requeue;
		}

		if (sc->rx.frag) {
			int space = skb->len - skb_tailroom(hdr_skb);

			if (pskb_expand_head(hdr_skb, 0, space, GFP_ATOMIC) < 0) {
				dev_kfree_skb(skb);
				RX_STAT_INC(rx_oom_err);
				goto requeue_drop_frag;
			}

			sc->rx.frag = NULL;

			skb_copy_from_linear_data(skb, skb_put(hdr_skb, skb->len),
						  skb->len);
			dev_kfree_skb_any(skb);
			skb = hdr_skb;
		}


		if (ah->caps.hw_caps & ATH9K_HW_CAP_ANT_DIV_COMB) {

			/*
			 * change the default rx antenna if rx diversity
			 * chooses the other antenna 3 times in a row.
			 */
			if (sc->rx.defant != rs.rs_antenna) {
				if (++sc->rx.rxotherant >= 3)
					ath_setdefantenna(sc, rs.rs_antenna);
			} else {
				sc->rx.rxotherant = 0;
			}

		}

		if (rxs->flag & RX_FLAG_MMIC_STRIPPED)
			skb_trim(skb, skb->len - 8);

		spin_lock_irqsave(&sc->sc_pm_lock, flags);
		if ((sc->ps_flags & (PS_WAIT_FOR_BEACON |
				     PS_WAIT_FOR_CAB |
				     PS_WAIT_FOR_PSPOLL_DATA)) ||
		    ath9k_check_auto_sleep(sc))
			ath_rx_ps(sc, skb, rs.is_mybeacon);
		spin_unlock_irqrestore(&sc->sc_pm_lock, flags);

		if ((ah->caps.hw_caps & ATH9K_HW_CAP_ANT_DIV_COMB) && sc->ant_rx == 3)
			ath_ant_comb_scan(sc, &rs);

		ieee80211_rx(hw, skb);

requeue_drop_frag:
		if (sc->rx.frag) {
			dev_kfree_skb_any(sc->rx.frag);
			sc->rx.frag = NULL;
		}
requeue:
		if (edma) {
			list_add_tail(&bf->list, &sc->rx.rxbuf);
			ath_rx_edma_buf_link(sc, qtype);
		} else {
			list_move_tail(&bf->list, &sc->rx.rxbuf);
			ath_rx_buf_link(sc, bf);
			if (!flush)
				ath9k_hw_rxena(ah);
		}
	} while (1);

	spin_unlock_bh(&sc->rx.rxbuflock);

	if (!(ah->imask & ATH9K_INT_RXEOL)) {
		ah->imask |= (ATH9K_INT_RXEOL | ATH9K_INT_RXORN);
		ath9k_hw_set_interrupts(ah);
	}

	return 0;
}