	 */
	spin_lock_bh(&ab->base_lock);
	peer = ath11k_peer_find(ab, arvif->vdev_id, peer_addr);

	/* flush the fragments cache during key (re)install to
	 * ensure all frags in the new frag list belong to the same key.
	 */
	if (peer && cmd == SET_KEY)
		ath11k_peer_frags_flush(ar, peer);
	spin_unlock_bh(&ab->base_lock);

	if (!peer) {
		if (cmd == SET_KEY) {