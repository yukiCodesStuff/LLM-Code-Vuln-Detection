	 * If there was a significant jump in the nssn - adjust.
	 * If the SN is smaller than the NSSN it might need to first go into
	 * the reorder buffer, in which case we just release up to it and the
	 * rest of the function will take of storing it and releasing up to the
	 * nssn
	 */
	if (!iwl_mvm_is_sn_less(nssn, buffer->head_sn + buffer->buf_size,
				buffer->buf_size)) {
		u16 min_sn = ieee80211_sn_less(sn, nssn) ? sn : nssn;

		iwl_mvm_release_frames(mvm, sta, napi, buffer, min_sn);
	}