	__skb_queue_purge(&rx_tid->rx_frags);
}

void ath11k_peer_frags_flush(struct ath11k *ar, struct ath11k_peer *peer)
{
	struct dp_rx_tid *rx_tid;
	int i;

	lockdep_assert_held(&ar->ab->base_lock);

	for (i = 0; i <= IEEE80211_NUM_TIDS; i++) {
		rx_tid = &peer->rx_tid[i];

		spin_unlock_bh(&ar->ab->base_lock);
		del_timer_sync(&rx_tid->frag_timer);
		spin_lock_bh(&ar->ab->base_lock);

		ath11k_dp_rx_frags_cleanup(rx_tid, true);
	}
}

void ath11k_peer_rx_tid_cleanup(struct ath11k *ar, struct ath11k_peer *peer)
{
	struct dp_rx_tid *rx_tid;
	int i;