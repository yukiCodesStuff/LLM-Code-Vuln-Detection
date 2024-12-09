
	rx_ring->bufs_max = num_entries;
	ath11k_dp_rxbufs_replenish(ar->ab, dp->mac_id, rx_ring, num_entries,
				   HAL_RX_BUF_RBM_SW3_BM);
	return 0;
}

static int ath11k_dp_rxdma_pdev_buf_setup(struct ath11k *ar)
		rx_ring = &ar->dp.rx_refill_buf_ring;

		ath11k_dp_rxbufs_replenish(ab, i, rx_ring, num_buffs_reaped[i],
					   HAL_RX_BUF_RBM_SW3_BM);
	}

	ath11k_dp_rx_process_received_packets(ab, napi, &msdu_list,
					      &quota, ring_id);
					     int *budget, struct sk_buff_head *skb_list)
{
	struct ath11k *ar;
	struct ath11k_pdev_dp *dp;
	struct dp_rxdma_ring *rx_ring;
	struct hal_srng *srng;
	void *rx_mon_status_desc;
							&buf_id);

		if (!skb) {
			ath11k_hal_rx_buf_addr_info_set(rx_mon_status_desc, 0, 0,
							HAL_RX_BUF_RBM_SW3_BM);
			num_buffs_reaped++;
			break;
		}
		rxcb = ATH11K_SKB_RXCB(skb);
			 FIELD_PREP(DP_RXDMA_BUF_COOKIE_BUF_ID, buf_id);

		ath11k_hal_rx_buf_addr_info_set(rx_mon_status_desc, rxcb->paddr,
						cookie, HAL_RX_BUF_RBM_SW3_BM);
		ath11k_hal_srng_src_get_next_entry(ab, srng);
		num_buffs_reaped++;
	}
	ath11k_hal_srng_access_end(ab, srng);
	cookie = FIELD_PREP(DP_RXDMA_BUF_COOKIE_PDEV_ID, dp->mac_id) |
		 FIELD_PREP(DP_RXDMA_BUF_COOKIE_BUF_ID, buf_id);

	ath11k_hal_rx_buf_addr_info_set(msdu0, paddr, cookie, HAL_RX_BUF_RBM_SW3_BM);

	/* Fill mpdu details into reo entrace ring */
	srng = &ab->hal.srng_list[ab->dp.reo_reinject_ring.ring_id];

		ath11k_hal_rx_msdu_link_info_get(link_desc_va, &num_msdus, msdu_cookies,
						 &rbm);
		if (rbm != HAL_RX_BUF_RBM_WBM_IDLE_DESC_LIST &&
		    rbm != HAL_RX_BUF_RBM_SW3_BM) {
			ab->soc_stats.invalid_rbm++;
			ath11k_warn(ab, "invalid return buffer manager %d\n", rbm);
			ath11k_dp_rx_link_desc_return(ab, desc,
						      HAL_WBM_REL_BM_ACT_REL_MSDU);
		rx_ring = &ar->dp.rx_refill_buf_ring;

		ath11k_dp_rxbufs_replenish(ab, i, rx_ring, n_bufs_reaped[i],
					   HAL_RX_BUF_RBM_SW3_BM);
	}

	return tot_n_bufs_reaped;
}
		rx_ring = &ar->dp.rx_refill_buf_ring;

		ath11k_dp_rxbufs_replenish(ab, i, rx_ring, num_buffs_reaped[i],
					   HAL_RX_BUF_RBM_SW3_BM);
	}

	rcu_read_lock();
	for (i = 0; i <  ab->num_radios; i++) {

	if (num_buf_freed)
		ath11k_dp_rxbufs_replenish(ab, mac_id, rx_ring, num_buf_freed,
					   HAL_RX_BUF_RBM_SW3_BM);

	return budget - quota;
}

{
	struct ath11k_pdev_dp *dp = &ar->dp;
	struct ath11k_mon_data *pmon = (struct ath11k_mon_data *)&dp->mon_data;
	void *ring_entry;
	void *mon_dst_srng;
	u32 ppdu_id;
	u32 rx_bufs_used;

	if (rx_bufs_used) {
		rx_mon_stats->dest_ppdu_done++;
		if (ar->ab->hw_params.rxdma1_enable)
			ath11k_dp_rxbufs_replenish(ar->ab, dp->mac_id,
						   &dp->rxdma_mon_buf_ring,
						   rx_bufs_used,
						   HAL_RX_BUF_RBM_SW3_BM);
		else
			ath11k_dp_rxbufs_replenish(ar->ab, dp->mac_id,
						   &dp->rx_refill_buf_ring,
						   rx_bufs_used,
						   HAL_RX_BUF_RBM_SW3_BM);
	}
}

static void ath11k_dp_rx_mon_status_process_tlv(struct ath11k *ar,