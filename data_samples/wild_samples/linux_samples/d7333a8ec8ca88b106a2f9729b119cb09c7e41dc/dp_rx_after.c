{
	struct ath11k_pdev_dp *dp = &ar->dp;
	int num_entries;

	num_entries = rx_ring->refill_buf_ring.size /
		ath11k_hal_srng_get_entrysize(ar->ab, ringtype);

	rx_ring->bufs_max = num_entries;
	ath11k_dp_rxbufs_replenish(ar->ab, dp->mac_id, rx_ring, num_entries,
				   ar->ab->hw_params.hal_params->rx_buf_rbm);
	return 0;
}

static int ath11k_dp_rxdma_pdev_buf_setup(struct ath11k *ar)
{
	struct ath11k_pdev_dp *dp = &ar->dp;
	struct ath11k_base *ab = ar->ab;
	struct dp_rxdma_ring *rx_ring = &dp->rx_refill_buf_ring;
	int i;

	ath11k_dp_rxdma_ring_buf_setup(ar, rx_ring, HAL_RXDMA_BUF);

	if (ar->ab->hw_params.rxdma1_enable) {
		rx_ring = &dp->rxdma_mon_buf_ring;
		ath11k_dp_rxdma_ring_buf_setup(ar, rx_ring, HAL_RXDMA_MONITOR_BUF);
	}
	for (i = 0; i < ab->num_radios; i++) {
		if (!num_buffs_reaped[i])
			continue;

		ar = ab->pdevs[i].ar;
		rx_ring = &ar->dp.rx_refill_buf_ring;

		ath11k_dp_rxbufs_replenish(ab, i, rx_ring, num_buffs_reaped[i],
					   ab->hw_params.hal_params->rx_buf_rbm);
	}

	ath11k_dp_rx_process_received_packets(ab, napi, &msdu_list,
					      &quota, ring_id);

exit:
	return budget - quota;
}

static void ath11k_dp_rx_update_peer_stats(struct ath11k_sta *arsta,
					   struct hal_rx_mon_ppdu_info *ppdu_info)
{
	struct ath11k_rx_peer_stats *rx_stats = arsta->rx_stats;
	u32 num_msdu;

	if (!rx_stats)
		return;

	num_msdu = ppdu_info->tcp_msdu_count + ppdu_info->tcp_ack_msdu_count +
		   ppdu_info->udp_msdu_count + ppdu_info->other_msdu_count;

	rx_stats->num_msdu += num_msdu;
	rx_stats->tcp_msdu_count += ppdu_info->tcp_msdu_count +
				    ppdu_info->tcp_ack_msdu_count;
	rx_stats->udp_msdu_count += ppdu_info->udp_msdu_count;
	rx_stats->other_msdu_count += ppdu_info->other_msdu_count;

	if (ppdu_info->preamble_type == HAL_RX_PREAMBLE_11A ||
	    ppdu_info->preamble_type == HAL_RX_PREAMBLE_11B) {
		ppdu_info->nss = 1;
		ppdu_info->mcs = HAL_RX_MAX_MCS;
		ppdu_info->tid = IEEE80211_NUM_TIDS;
	}

	if (ppdu_info->nss > 0 && ppdu_info->nss <= HAL_RX_MAX_NSS)
		rx_stats->nss_count[ppdu_info->nss - 1] += num_msdu;

	if (ppdu_info->mcs <= HAL_RX_MAX_MCS)
		rx_stats->mcs_count[ppdu_info->mcs] += num_msdu;

	if (ppdu_info->gi < HAL_RX_GI_MAX)
		rx_stats->gi_count[ppdu_info->gi] += num_msdu;

	if (ppdu_info->bw < HAL_RX_BW_MAX)
		rx_stats->bw_count[ppdu_info->bw] += num_msdu;

	if (ppdu_info->ldpc < HAL_RX_SU_MU_CODING_MAX)
		rx_stats->coding_count[ppdu_info->ldpc] += num_msdu;

	if (ppdu_info->tid <= IEEE80211_NUM_TIDS)
		rx_stats->tid_count[ppdu_info->tid] += num_msdu;

	if (ppdu_info->preamble_type < HAL_RX_PREAMBLE_MAX)
		rx_stats->pream_cnt[ppdu_info->preamble_type] += num_msdu;

	if (ppdu_info->reception_type < HAL_RX_RECEPTION_TYPE_MAX)
		rx_stats->reception_type[ppdu_info->reception_type] += num_msdu;

	if (ppdu_info->is_stbc)
		rx_stats->stbc_count += num_msdu;

	if (ppdu_info->beamformed)
		rx_stats->beamformed_count += num_msdu;

	if (ppdu_info->num_mpdu_fcs_ok > 1)
		rx_stats->ampdu_msdu_count += num_msdu;
	else
		rx_stats->non_ampdu_msdu_count += num_msdu;

	rx_stats->num_mpdu_fcs_ok += ppdu_info->num_mpdu_fcs_ok;
	rx_stats->num_mpdu_fcs_err += ppdu_info->num_mpdu_fcs_err;
	rx_stats->dcm_count += ppdu_info->dcm;
	rx_stats->ru_alloc_cnt[ppdu_info->ru_alloc] += num_msdu;

	arsta->rssi_comb = ppdu_info->rssi_comb;
	rx_stats->rx_duration += ppdu_info->rx_duration;
	arsta->rx_duration = rx_stats->rx_duration;
}

static struct sk_buff *ath11k_dp_rx_alloc_mon_status_buf(struct ath11k_base *ab,
							 struct dp_rxdma_ring *rx_ring,
							 int *buf_id)
{
	struct sk_buff *skb;
	dma_addr_t paddr;

	skb = dev_alloc_skb(DP_RX_BUFFER_SIZE +
			    DP_RX_BUFFER_ALIGN_SIZE);

	if (!skb)
		goto fail_alloc_skb;

	if (!IS_ALIGNED((unsigned long)skb->data,
			DP_RX_BUFFER_ALIGN_SIZE)) {
		skb_pull(skb, PTR_ALIGN(skb->data, DP_RX_BUFFER_ALIGN_SIZE) -
			 skb->data);
	}

	paddr = dma_map_single(ab->dev, skb->data,
			       skb->len + skb_tailroom(skb),
			       DMA_FROM_DEVICE);
	if (unlikely(dma_mapping_error(ab->dev, paddr)))
		goto fail_free_skb;

	spin_lock_bh(&rx_ring->idr_lock);
	*buf_id = idr_alloc(&rx_ring->bufs_idr, skb, 0,
			    rx_ring->bufs_max, GFP_ATOMIC);
	spin_unlock_bh(&rx_ring->idr_lock);
	if (*buf_id < 0)
		goto fail_dma_unmap;

	ATH11K_SKB_RXCB(skb)->paddr = paddr;
	return skb;

fail_dma_unmap:
	dma_unmap_single(ab->dev, paddr, skb->len + skb_tailroom(skb),
			 DMA_FROM_DEVICE);
fail_free_skb:
	dev_kfree_skb_any(skb);
fail_alloc_skb:
	return NULL;
}

int ath11k_dp_rx_mon_status_bufs_replenish(struct ath11k_base *ab, int mac_id,
					   struct dp_rxdma_ring *rx_ring,
					   int req_entries,
					   enum hal_rx_buf_return_buf_manager mgr)
{
	struct hal_srng *srng;
	u32 *desc;
	struct sk_buff *skb;
	int num_free;
	int num_remain;
	int buf_id;
	u32 cookie;
	dma_addr_t paddr;

	req_entries = min(req_entries, rx_ring->bufs_max);

	srng = &ab->hal.srng_list[rx_ring->refill_buf_ring.ring_id];

	spin_lock_bh(&srng->lock);

	ath11k_hal_srng_access_begin(ab, srng);

	num_free = ath11k_hal_srng_src_num_free(ab, srng, true);

	req_entries = min(num_free, req_entries);
	num_remain = req_entries;

	while (num_remain > 0) {
		skb = ath11k_dp_rx_alloc_mon_status_buf(ab, rx_ring,
							&buf_id);
		if (!skb)
			break;
		paddr = ATH11K_SKB_RXCB(skb)->paddr;

		desc = ath11k_hal_srng_src_get_next_entry(ab, srng);
		if (!desc)
			goto fail_desc_get;

		cookie = FIELD_PREP(DP_RXDMA_BUF_COOKIE_PDEV_ID, mac_id) |
			 FIELD_PREP(DP_RXDMA_BUF_COOKIE_BUF_ID, buf_id);

		num_remain--;

		ath11k_hal_rx_buf_addr_info_set(desc, paddr, cookie, mgr);
	}

	ath11k_hal_srng_access_end(ab, srng);

	spin_unlock_bh(&srng->lock);

	return req_entries - num_remain;

fail_desc_get:
	spin_lock_bh(&rx_ring->idr_lock);
	idr_remove(&rx_ring->bufs_idr, buf_id);
	spin_unlock_bh(&rx_ring->idr_lock);
	dma_unmap_single(ab->dev, paddr, skb->len + skb_tailroom(skb),
			 DMA_FROM_DEVICE);
	dev_kfree_skb_any(skb);
	ath11k_hal_srng_access_end(ab, srng);
	spin_unlock_bh(&srng->lock);

	return req_entries - num_remain;
}

static int ath11k_dp_rx_reap_mon_status_ring(struct ath11k_base *ab, int mac_id,
					     int *budget, struct sk_buff_head *skb_list)
{
	struct ath11k *ar;
	const struct ath11k_hw_hal_params *hal_params;
	struct ath11k_pdev_dp *dp;
	struct dp_rxdma_ring *rx_ring;
	struct hal_srng *srng;
	void *rx_mon_status_desc;
	struct sk_buff *skb;
	struct ath11k_skb_rxcb *rxcb;
	struct hal_tlv_hdr *tlv;
	u32 cookie;
	int buf_id, srng_id;
	dma_addr_t paddr;
	u8 rbm;
	int num_buffs_reaped = 0;

	ar = ab->pdevs[ath11k_hw_mac_id_to_pdev_id(&ab->hw_params, mac_id)].ar;
	dp = &ar->dp;
	srng_id = ath11k_hw_mac_id_to_srng_id(&ab->hw_params, mac_id);
	rx_ring = &dp->rx_mon_status_refill_ring[srng_id];

	srng = &ab->hal.srng_list[rx_ring->refill_buf_ring.ring_id];

	spin_lock_bh(&srng->lock);

	ath11k_hal_srng_access_begin(ab, srng);
	while (*budget) {
		*budget -= 1;
		rx_mon_status_desc =
			ath11k_hal_srng_src_peek(ab, srng);
		if (!rx_mon_status_desc)
			break;

		ath11k_hal_rx_buf_addr_info_get(rx_mon_status_desc, &paddr,
						&cookie, &rbm);
		if (paddr) {
			buf_id = FIELD_GET(DP_RXDMA_BUF_COOKIE_BUF_ID, cookie);

			spin_lock_bh(&rx_ring->idr_lock);
			skb = idr_find(&rx_ring->bufs_idr, buf_id);
			if (!skb) {
				ath11k_warn(ab, "rx monitor status with invalid buf_id %d\n",
					    buf_id);
				spin_unlock_bh(&rx_ring->idr_lock);
				goto move_next;
			}

			idr_remove(&rx_ring->bufs_idr, buf_id);
			spin_unlock_bh(&rx_ring->idr_lock);

			rxcb = ATH11K_SKB_RXCB(skb);

			dma_unmap_single(ab->dev, rxcb->paddr,
					 skb->len + skb_tailroom(skb),
					 DMA_FROM_DEVICE);

			tlv = (struct hal_tlv_hdr *)skb->data;
			if (FIELD_GET(HAL_TLV_HDR_TAG, tlv->tl) !=
					HAL_RX_STATUS_BUFFER_DONE) {
				ath11k_warn(ab, "mon status DONE not set %lx\n",
					    FIELD_GET(HAL_TLV_HDR_TAG,
						      tlv->tl));
				dev_kfree_skb_any(skb);
				goto move_next;
			}

			__skb_queue_tail(skb_list, skb);
		}
{
	struct ath11k *ar;
	const struct ath11k_hw_hal_params *hal_params;
	struct ath11k_pdev_dp *dp;
	struct dp_rxdma_ring *rx_ring;
	struct hal_srng *srng;
	void *rx_mon_status_desc;
	struct sk_buff *skb;
	struct ath11k_skb_rxcb *rxcb;
	struct hal_tlv_hdr *tlv;
	u32 cookie;
	int buf_id, srng_id;
	dma_addr_t paddr;
	u8 rbm;
	int num_buffs_reaped = 0;

	ar = ab->pdevs[ath11k_hw_mac_id_to_pdev_id(&ab->hw_params, mac_id)].ar;
	dp = &ar->dp;
	srng_id = ath11k_hw_mac_id_to_srng_id(&ab->hw_params, mac_id);
	rx_ring = &dp->rx_mon_status_refill_ring[srng_id];

	srng = &ab->hal.srng_list[rx_ring->refill_buf_ring.ring_id];

	spin_lock_bh(&srng->lock);

	ath11k_hal_srng_access_begin(ab, srng);
	while (*budget) {
		*budget -= 1;
		rx_mon_status_desc =
			ath11k_hal_srng_src_peek(ab, srng);
		if (!rx_mon_status_desc)
			break;

		ath11k_hal_rx_buf_addr_info_get(rx_mon_status_desc, &paddr,
						&cookie, &rbm);
		if (paddr) {
			buf_id = FIELD_GET(DP_RXDMA_BUF_COOKIE_BUF_ID, cookie);

			spin_lock_bh(&rx_ring->idr_lock);
			skb = idr_find(&rx_ring->bufs_idr, buf_id);
			if (!skb) {
				ath11k_warn(ab, "rx monitor status with invalid buf_id %d\n",
					    buf_id);
				spin_unlock_bh(&rx_ring->idr_lock);
				goto move_next;
			}

			idr_remove(&rx_ring->bufs_idr, buf_id);
			spin_unlock_bh(&rx_ring->idr_lock);

			rxcb = ATH11K_SKB_RXCB(skb);

			dma_unmap_single(ab->dev, rxcb->paddr,
					 skb->len + skb_tailroom(skb),
					 DMA_FROM_DEVICE);

			tlv = (struct hal_tlv_hdr *)skb->data;
			if (FIELD_GET(HAL_TLV_HDR_TAG, tlv->tl) !=
					HAL_RX_STATUS_BUFFER_DONE) {
				ath11k_warn(ab, "mon status DONE not set %lx\n",
					    FIELD_GET(HAL_TLV_HDR_TAG,
						      tlv->tl));
				dev_kfree_skb_any(skb);
				goto move_next;
			}

			__skb_queue_tail(skb_list, skb);
		}
move_next:
		skb = ath11k_dp_rx_alloc_mon_status_buf(ab, rx_ring,
							&buf_id);

		if (!skb) {
			hal_params = ab->hw_params.hal_params;
			ath11k_hal_rx_buf_addr_info_set(rx_mon_status_desc, 0, 0,
							hal_params->rx_buf_rbm);
			num_buffs_reaped++;
			break;
		}
		rxcb = ATH11K_SKB_RXCB(skb);

		cookie = FIELD_PREP(DP_RXDMA_BUF_COOKIE_PDEV_ID, mac_id) |
			 FIELD_PREP(DP_RXDMA_BUF_COOKIE_BUF_ID, buf_id);

		ath11k_hal_rx_buf_addr_info_set(rx_mon_status_desc, rxcb->paddr,
						cookie,
						ab->hw_params.hal_params->rx_buf_rbm);
		ath11k_hal_srng_src_get_next_entry(ab, srng);
		num_buffs_reaped++;
	}
	ath11k_hal_srng_access_end(ab, srng);
	spin_unlock_bh(&srng->lock);

	return num_buffs_reaped;
}
					HAL_RX_STATUS_BUFFER_DONE) {
				ath11k_warn(ab, "mon status DONE not set %lx\n",
					    FIELD_GET(HAL_TLV_HDR_TAG,
						      tlv->tl));
				dev_kfree_skb_any(skb);
				goto move_next;
			}

			__skb_queue_tail(skb_list, skb);
		}
move_next:
		skb = ath11k_dp_rx_alloc_mon_status_buf(ab, rx_ring,
							&buf_id);

		if (!skb) {
			hal_params = ab->hw_params.hal_params;
			ath11k_hal_rx_buf_addr_info_set(rx_mon_status_desc, 0, 0,
							hal_params->rx_buf_rbm);
			num_buffs_reaped++;
			break;
		}
		if (!skb) {
			hal_params = ab->hw_params.hal_params;
			ath11k_hal_rx_buf_addr_info_set(rx_mon_status_desc, 0, 0,
							hal_params->rx_buf_rbm);
			num_buffs_reaped++;
			break;
		}
		rxcb = ATH11K_SKB_RXCB(skb);

		cookie = FIELD_PREP(DP_RXDMA_BUF_COOKIE_PDEV_ID, mac_id) |
			 FIELD_PREP(DP_RXDMA_BUF_COOKIE_BUF_ID, buf_id);

		ath11k_hal_rx_buf_addr_info_set(rx_mon_status_desc, rxcb->paddr,
						cookie,
						ab->hw_params.hal_params->rx_buf_rbm);
		ath11k_hal_srng_src_get_next_entry(ab, srng);
		num_buffs_reaped++;
	}
	ath11k_hal_srng_access_end(ab, srng);
	spin_unlock_bh(&srng->lock);

	return num_buffs_reaped;
}

int ath11k_dp_rx_process_mon_status(struct ath11k_base *ab, int mac_id,
				    struct napi_struct *napi, int budget)
{
	struct ath11k *ar = ath11k_ab_to_ar(ab, mac_id);
	enum hal_rx_mon_status hal_status;
	struct sk_buff *skb;
	struct sk_buff_head skb_list;
	struct hal_rx_mon_ppdu_info ppdu_info;
	struct ath11k_peer *peer;
	struct ath11k_sta *arsta;
	int num_buffs_reaped = 0;
	u32 rx_buf_sz;
	u16 log_type = 0;

	__skb_queue_head_init(&skb_list);

	num_buffs_reaped = ath11k_dp_rx_reap_mon_status_ring(ab, mac_id, &budget,
							     &skb_list);
	if (!num_buffs_reaped)
		goto exit;

	while ((skb = __skb_dequeue(&skb_list))) {
		memset(&ppdu_info, 0, sizeof(ppdu_info));
		ppdu_info.peer_id = HAL_INVALID_PEERID;

		if (ath11k_debugfs_is_pktlog_lite_mode_enabled(ar)) {
			log_type = ATH11K_PKTLOG_TYPE_LITE_RX;
			rx_buf_sz = DP_RX_BUFFER_SIZE_LITE;
		} else if (ath11k_debugfs_is_pktlog_rx_stats_enabled(ar)) {
			log_type = ATH11K_PKTLOG_TYPE_RX_STATBUF;
			rx_buf_sz = DP_RX_BUFFER_SIZE;
		}

		if (log_type)
			trace_ath11k_htt_rxdesc(ar, skb->data, log_type, rx_buf_sz);

		hal_status = ath11k_hal_rx_parse_mon_status(ab, &ppdu_info, skb);

		if (ppdu_info.peer_id == HAL_INVALID_PEERID ||
		    hal_status != HAL_RX_MON_STATUS_PPDU_DONE) {
			dev_kfree_skb_any(skb);
			continue;
		}

		rcu_read_lock();
		spin_lock_bh(&ab->base_lock);
		peer = ath11k_peer_find_by_id(ab, ppdu_info.peer_id);

		if (!peer || !peer->sta) {
			ath11k_dbg(ab, ATH11K_DBG_DATA,
				   "failed to find the peer with peer_id %d\n",
				   ppdu_info.peer_id);
			spin_unlock_bh(&ab->base_lock);
			rcu_read_unlock();
			dev_kfree_skb_any(skb);
			continue;
		}

		arsta = (struct ath11k_sta *)peer->sta->drv_priv;
		ath11k_dp_rx_update_peer_stats(arsta, &ppdu_info);

		if (ath11k_debugfs_is_pktlog_peer_valid(ar, peer->addr))
			trace_ath11k_htt_rxdesc(ar, skb->data, log_type, rx_buf_sz);

		spin_unlock_bh(&ab->base_lock);
		rcu_read_unlock();

		dev_kfree_skb_any(skb);
	}
	if (buf_id < 0) {
		ret = -ENOMEM;
		goto err_unmap_dma;
	}

	ATH11K_SKB_RXCB(defrag_skb)->paddr = paddr;
	cookie = FIELD_PREP(DP_RXDMA_BUF_COOKIE_PDEV_ID, dp->mac_id) |
		 FIELD_PREP(DP_RXDMA_BUF_COOKIE_BUF_ID, buf_id);

	ath11k_hal_rx_buf_addr_info_set(msdu0, paddr, cookie,
					ab->hw_params.hal_params->rx_buf_rbm);

	/* Fill mpdu details into reo entrace ring */
	srng = &ab->hal.srng_list[ab->dp.reo_reinject_ring.ring_id];

	spin_lock_bh(&srng->lock);
	ath11k_hal_srng_access_begin(ab, srng);

	reo_ent_ring = (struct hal_reo_entrance_ring *)
			ath11k_hal_srng_src_get_next_entry(ab, srng);
	if (!reo_ent_ring) {
		ath11k_hal_srng_access_end(ab, srng);
		spin_unlock_bh(&srng->lock);
		ret = -ENOSPC;
		goto err_free_idr;
	}
		if (ret) {
			ath11k_warn(ab, "failed to parse error reo desc %d\n",
				    ret);
			continue;
		}
		link_desc_va = link_desc_banks[desc_bank].vaddr +
			       (paddr - link_desc_banks[desc_bank].paddr);
		ath11k_hal_rx_msdu_link_info_get(link_desc_va, &num_msdus, msdu_cookies,
						 &rbm);
		if (rbm != HAL_RX_BUF_RBM_WBM_IDLE_DESC_LIST &&
		    rbm != ab->hw_params.hal_params->rx_buf_rbm) {
			ab->soc_stats.invalid_rbm++;
			ath11k_warn(ab, "invalid return buffer manager %d\n", rbm);
			ath11k_dp_rx_link_desc_return(ab, desc,
						      HAL_WBM_REL_BM_ACT_REL_MSDU);
			continue;
		}
	for (i = 0; i <  ab->num_radios; i++) {
		if (!n_bufs_reaped[i])
			continue;

		ar = ab->pdevs[i].ar;
		rx_ring = &ar->dp.rx_refill_buf_ring;

		ath11k_dp_rxbufs_replenish(ab, i, rx_ring, n_bufs_reaped[i],
					   ab->hw_params.hal_params->rx_buf_rbm);
	}

	return tot_n_bufs_reaped;
}

static void ath11k_dp_rx_null_q_desc_sg_drop(struct ath11k *ar,
					     int msdu_len,
					     struct sk_buff_head *msdu_list)
{
	struct sk_buff *skb, *tmp;
	struct ath11k_skb_rxcb *rxcb;
	int n_buffs;

	n_buffs = DIV_ROUND_UP(msdu_len,
			       (DP_RX_BUFFER_SIZE - ar->ab->hw_params.hal_desc_sz));

	skb_queue_walk_safe(msdu_list, skb, tmp) {
		rxcb = ATH11K_SKB_RXCB(skb);
		if (rxcb->err_rel_src == HAL_WBM_REL_SRC_MODULE_REO &&
		    rxcb->err_code == HAL_REO_DEST_RING_ERROR_CODE_DESC_ADDR_ZERO) {
			if (!n_buffs)
				break;
			__skb_unlink(skb, msdu_list);
			dev_kfree_skb_any(skb);
			n_buffs--;
		}
	for (i = 0; i <  ab->num_radios; i++) {
		if (!num_buffs_reaped[i])
			continue;

		ar = ab->pdevs[i].ar;
		rx_ring = &ar->dp.rx_refill_buf_ring;

		ath11k_dp_rxbufs_replenish(ab, i, rx_ring, num_buffs_reaped[i],
					   ab->hw_params.hal_params->rx_buf_rbm);
	}

	rcu_read_lock();
	for (i = 0; i <  ab->num_radios; i++) {
		if (!rcu_dereference(ab->pdevs_active[i])) {
			__skb_queue_purge(&msdu_list[i]);
			continue;
		}
			if (!skb) {
				ath11k_warn(ab, "rxdma error with invalid buf_id %d\n",
					    buf_id);
				spin_unlock_bh(&rx_ring->idr_lock);
				continue;
			}

			idr_remove(&rx_ring->bufs_idr, buf_id);
			spin_unlock_bh(&rx_ring->idr_lock);

			rxcb = ATH11K_SKB_RXCB(skb);
			dma_unmap_single(ab->dev, rxcb->paddr,
					 skb->len + skb_tailroom(skb),
					 DMA_FROM_DEVICE);
			dev_kfree_skb_any(skb);

			num_buf_freed++;
		}

		ath11k_dp_rx_link_desc_return(ab, desc,
					      HAL_WBM_REL_BM_ACT_PUT_IN_IDLE);
	}

	ath11k_hal_srng_access_end(ab, srng);

	spin_unlock_bh(&srng->lock);

	if (num_buf_freed)
		ath11k_dp_rxbufs_replenish(ab, mac_id, rx_ring, num_buf_freed,
					   ab->hw_params.hal_params->rx_buf_rbm);

	return budget - quota;
}

void ath11k_dp_process_reo_status(struct ath11k_base *ab)
{
	struct ath11k_dp *dp = &ab->dp;
	struct hal_srng *srng;
	struct dp_reo_cmd *cmd, *tmp;
	bool found = false;
	u32 *reo_desc;
	u16 tag;
	struct hal_reo_status reo_status;

	srng = &ab->hal.srng_list[dp->reo_status_ring.ring_id];

	memset(&reo_status, 0, sizeof(reo_status));

	spin_lock_bh(&srng->lock);

	ath11k_hal_srng_access_begin(ab, srng);

	while ((reo_desc = ath11k_hal_srng_dst_get_next_entry(ab, srng))) {
		tag = FIELD_GET(HAL_SRNG_TLV_HDR_TAG, *reo_desc);

		switch (tag) {
		case HAL_REO_GET_QUEUE_STATS_STATUS:
			ath11k_hal_reo_status_queue_stats(ab, reo_desc,
							  &reo_status);
			break;
		case HAL_REO_FLUSH_QUEUE_STATUS:
			ath11k_hal_reo_flush_queue_status(ab, reo_desc,
							  &reo_status);
			break;
		case HAL_REO_FLUSH_CACHE_STATUS:
			ath11k_hal_reo_flush_cache_status(ab, reo_desc,
							  &reo_status);
			break;
		case HAL_REO_UNBLOCK_CACHE_STATUS:
			ath11k_hal_reo_unblk_cache_status(ab, reo_desc,
							  &reo_status);
			break;
		case HAL_REO_FLUSH_TIMEOUT_LIST_STATUS:
			ath11k_hal_reo_flush_timeout_list_status(ab, reo_desc,
								 &reo_status);
			break;
		case HAL_REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS:
			ath11k_hal_reo_desc_thresh_reached_status(ab, reo_desc,
								  &reo_status);
			break;
		case HAL_REO_UPDATE_RX_REO_QUEUE_STATUS:
			ath11k_hal_reo_update_rx_reo_queue_status(ab, reo_desc,
								  &reo_status);
			break;
		default:
			ath11k_warn(ab, "Unknown reo status type %d\n", tag);
			continue;
		}

		spin_lock_bh(&dp->reo_cmd_lock);
		list_for_each_entry_safe(cmd, tmp, &dp->reo_cmd_list, list) {
			if (reo_status.uniform_hdr.cmd_num == cmd->cmd_num) {
				found = true;
				list_del(&cmd->list);
				break;
			}
		}
		spin_unlock_bh(&dp->reo_cmd_lock);

		if (found) {
			cmd->handler(dp, (void *)&cmd->data,
				     reo_status.uniform_hdr.cmd_status);
			kfree(cmd);
		}

		found = false;
	}
{
	struct ath11k_pdev_dp *dp = &ar->dp;
	struct ath11k_mon_data *pmon = (struct ath11k_mon_data *)&dp->mon_data;
	const struct ath11k_hw_hal_params *hal_params;
	void *ring_entry;
	void *mon_dst_srng;
	u32 ppdu_id;
	u32 rx_bufs_used;
	u32 ring_id;
	struct ath11k_pdev_mon_stats *rx_mon_stats;
	u32	 npackets = 0;

	if (ar->ab->hw_params.rxdma1_enable)
		ring_id = dp->rxdma_mon_dst_ring.ring_id;
	else
		ring_id = dp->rxdma_err_dst_ring[mac_id].ring_id;

	mon_dst_srng = &ar->ab->hal.srng_list[ring_id];

	if (!mon_dst_srng) {
		ath11k_warn(ar->ab,
			    "HAL Monitor Destination Ring Init Failed -- %pK",
			    mon_dst_srng);
		return;
	}
	if (rx_bufs_used) {
		rx_mon_stats->dest_ppdu_done++;
		hal_params = ar->ab->hw_params.hal_params;

		if (ar->ab->hw_params.rxdma1_enable)
			ath11k_dp_rxbufs_replenish(ar->ab, dp->mac_id,
						   &dp->rxdma_mon_buf_ring,
						   rx_bufs_used,
						   hal_params->rx_buf_rbm);
		else
			ath11k_dp_rxbufs_replenish(ar->ab, dp->mac_id,
						   &dp->rx_refill_buf_ring,
						   rx_bufs_used,
						   hal_params->rx_buf_rbm);
	}