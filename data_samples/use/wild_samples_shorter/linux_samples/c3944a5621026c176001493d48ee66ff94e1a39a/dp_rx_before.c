	__skb_queue_purge(&rx_tid->rx_frags);
}

void ath11k_peer_rx_tid_cleanup(struct ath11k *ar, struct ath11k_peer *peer)
{
	struct dp_rx_tid *rx_tid;
	int i;