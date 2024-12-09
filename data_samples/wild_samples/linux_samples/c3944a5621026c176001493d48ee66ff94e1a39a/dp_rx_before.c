	if (rx_tid->dst_ring_desc) {
		if (rel_link_desc)
			ath11k_dp_rx_link_desc_return(ab, (u32 *)rx_tid->dst_ring_desc,
						      HAL_WBM_REL_BM_ACT_PUT_IN_IDLE);
		kfree(rx_tid->dst_ring_desc);
		rx_tid->dst_ring_desc = NULL;
	}

	rx_tid->cur_sn = 0;
	rx_tid->last_frag_no = 0;
	rx_tid->rx_frag_bitmap = 0;
	__skb_queue_purge(&rx_tid->rx_frags);
}

void ath11k_peer_rx_tid_cleanup(struct ath11k *ar, struct ath11k_peer *peer)
{
	struct dp_rx_tid *rx_tid;
	int i;

	lockdep_assert_held(&ar->ab->base_lock);

	for (i = 0; i <= IEEE80211_NUM_TIDS; i++) {
		rx_tid = &peer->rx_tid[i];

		ath11k_peer_rx_tid_delete(ar, peer, i);
		ath11k_dp_rx_frags_cleanup(rx_tid, true);

		spin_unlock_bh(&ar->ab->base_lock);
		del_timer_sync(&rx_tid->frag_timer);
		spin_lock_bh(&ar->ab->base_lock);
	}