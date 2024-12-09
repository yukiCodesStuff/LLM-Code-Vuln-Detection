{
	int rate_idx = -1;
	u8 rate_plcp;
	u32 rate_flags = 0;
	bool is_cck;
	struct iwl_mvm_sta *mvmsta = iwl_mvm_sta_from_mac80211(sta);

	/* info->control is only relevant for non HW rate control */
	if (!ieee80211_hw_check(mvm->hw, HAS_RATE_CONTROL)) {
		/* HT rate doesn't make sense for a non data frame */
		WARN_ONCE(info->control.rates[0].flags & IEEE80211_TX_RC_MCS &&
			  !ieee80211_is_data(fc),
			  "Got a HT rate (flags:0x%x/mcs:%d/fc:0x%x/state:%d) for a non data frame\n",
			  info->control.rates[0].flags,
			  info->control.rates[0].idx,
			  le16_to_cpu(fc), mvmsta->sta_state);

		rate_idx = info->control.rates[0].idx;
	}
	if (!ieee80211_hw_check(mvm->hw, HAS_RATE_CONTROL)) {
		/* HT rate doesn't make sense for a non data frame */
		WARN_ONCE(info->control.rates[0].flags & IEEE80211_TX_RC_MCS &&
			  !ieee80211_is_data(fc),
			  "Got a HT rate (flags:0x%x/mcs:%d/fc:0x%x/state:%d) for a non data frame\n",
			  info->control.rates[0].flags,
			  info->control.rates[0].idx,
			  le16_to_cpu(fc), mvmsta->sta_state);

		rate_idx = info->control.rates[0].idx;
	}

	/* if the rate isn't a well known legacy rate, take the lowest one */
	if (rate_idx < 0 || rate_idx >= IWL_RATE_COUNT_LEGACY)
		rate_idx = rate_lowest_index(
				&mvm->nvm_data->bands[info->band], sta);

	/*
	 * For non 2 GHZ band, remap mac80211 rate
	 * indices into driver indices
	 */
	if (info->band != NL80211_BAND_2GHZ)
		rate_idx += IWL_FIRST_OFDM_RATE;

	/* For 2.4 GHZ band, check that there is no need to remap */
	BUILD_BUG_ON(IWL_FIRST_CCK_RATE != 0);

	/* Get PLCP rate for tx_cmd->rate_n_flags */
	rate_plcp = iwl_mvm_mac80211_idx_to_hwrate(mvm->fw, rate_idx);
	is_cck = (rate_idx >= IWL_FIRST_CCK_RATE) && (rate_idx <= IWL_LAST_CCK_RATE);

	/* Set CCK or OFDM flag */
	if (iwl_fw_lookup_cmd_ver(mvm->fw, LONG_GROUP, TX_CMD, 0) > 8) {
		if (!is_cck)
			rate_flags |= RATE_MCS_LEGACY_OFDM_MSK;
		else
			rate_flags |= RATE_MCS_CCK_MSK;
	} else if (is_cck) {
		rate_flags |= RATE_MCS_CCK_MSK_V1;
	}

	return (u32)rate_plcp | rate_flags;
}
	switch (status & TX_STATUS_MSK) {
	case TX_STATUS_SUCCESS:
		return "SUCCESS";
	TX_STATUS_POSTPONE(DELAY);
	TX_STATUS_POSTPONE(FEW_BYTES);
	TX_STATUS_POSTPONE(BT_PRIO);
	TX_STATUS_POSTPONE(QUIET_PERIOD);
	TX_STATUS_POSTPONE(CALC_TTAK);
	TX_STATUS_FAIL(INTERNAL_CROSSED_RETRY);
	TX_STATUS_FAIL(SHORT_LIMIT);
	TX_STATUS_FAIL(LONG_LIMIT);
	TX_STATUS_FAIL(UNDERRUN);
	TX_STATUS_FAIL(DRAIN_FLOW);
	TX_STATUS_FAIL(RFKILL_FLUSH);
	TX_STATUS_FAIL(LIFE_EXPIRE);
	TX_STATUS_FAIL(DEST_PS);
	TX_STATUS_FAIL(HOST_ABORTED);
	TX_STATUS_FAIL(BT_RETRY);
	TX_STATUS_FAIL(STA_INVALID);
	TX_STATUS_FAIL(FRAG_DROPPED);
	TX_STATUS_FAIL(TID_DISABLE);
	TX_STATUS_FAIL(FIFO_FLUSHED);
	TX_STATUS_FAIL(SMALL_CF_POLL);
	TX_STATUS_FAIL(FW_DROP);
	TX_STATUS_FAIL(STA_COLOR_MISMATCH);
	}

	return "UNKNOWN";

#undef TX_STATUS_FAIL
#undef TX_STATUS_POSTPONE
}
#endif /* CONFIG_IWLWIFI_DEBUG */

static int iwl_mvm_get_hwrate_chan_width(u32 chan_width)
{
	switch (chan_width) {
	case RATE_MCS_CHAN_WIDTH_20:
		return 0;
	case RATE_MCS_CHAN_WIDTH_40:
		return IEEE80211_TX_RC_40_MHZ_WIDTH;
	case RATE_MCS_CHAN_WIDTH_80:
		return IEEE80211_TX_RC_80_MHZ_WIDTH;
	case RATE_MCS_CHAN_WIDTH_160:
		return IEEE80211_TX_RC_160_MHZ_WIDTH;
	default:
		return 0;
	}
}
	} else {
		r->idx = iwl_mvm_legacy_rate_to_mac80211_idx(rate_n_flags,
							     band);
	}
}

/*
 * translate ucode response to mac80211 tx status control values
 */
static void iwl_mvm_hwrate_to_tx_status(const struct iwl_fw *fw,
					u32 rate_n_flags,
					struct ieee80211_tx_info *info)
{
	struct ieee80211_tx_rate *r = &info->status.rates[0];

	if (iwl_fw_lookup_notif_ver(fw, LONG_GROUP,
				    TX_CMD, 0) > 6)
		rate_n_flags = iwl_new_rate_from_v1(rate_n_flags);

	info->status.antenna =
		((rate_n_flags & RATE_MCS_ANT_AB_MSK) >> RATE_MCS_ANT_POS);
	iwl_mvm_hwrate_to_tx_rate(rate_n_flags,
				  info->band, r);
}
		switch (status & TX_STATUS_MSK) {
		case TX_STATUS_SUCCESS:
		case TX_STATUS_DIRECT_DONE:
			info->flags |= IEEE80211_TX_STAT_ACK;
			break;
		case TX_STATUS_FAIL_FIFO_FLUSHED:
		case TX_STATUS_FAIL_DRAIN_FLOW:
			flushed = true;
			break;
		case TX_STATUS_FAIL_DEST_PS:
			/* the FW should have stopped the queue and not
			 * return this status
			 */
			IWL_ERR_LIMIT(mvm,
				      "FW reported TX filtered, status=0x%x, FC=0x%x\n",
				      status, le16_to_cpu(hdr->frame_control));
			info->flags |= IEEE80211_TX_STAT_TX_FILTERED;
			break;
		default:
			break;
		}

		if ((status & TX_STATUS_MSK) != TX_STATUS_SUCCESS &&
		    ieee80211_is_mgmt(hdr->frame_control))
			iwl_mvm_toggle_tx_ant(mvm, &mvm->mgmt_last_antenna_idx);

		/*
		 * If we are freeing multiple frames, mark all the frames
		 * but the first one as acked, since they were acknowledged
		 * before
		 * */
		if (skb_freed > 1)
			info->flags |= IEEE80211_TX_STAT_ACK;

		iwl_mvm_tx_status_check_trigger(mvm, status, hdr->frame_control);

		info->status.rates[0].count = tx_resp->failure_frame + 1;

		iwl_mvm_hwrate_to_tx_status(mvm->fw,
					    le32_to_cpu(tx_resp->initial_rate),
					    info);

		/* Don't assign the converted initial_rate, because driver
		 * TLC uses this and doesn't support the new FW rate
		 */
		info->status.status_driver_data[1] =
			(void *)(uintptr_t)le32_to_cpu(tx_resp->initial_rate);

		/* Single frame failure in an AMPDU queue => send BAR */
		if (info->flags & IEEE80211_TX_CTL_AMPDU &&
		    !(info->flags & IEEE80211_TX_STAT_ACK) &&
		    !(info->flags & IEEE80211_TX_STAT_TX_FILTERED) && !flushed)
			info->flags |= IEEE80211_TX_STAT_AMPDU_NO_BACK;
		info->flags &= ~IEEE80211_TX_CTL_AMPDU;

		/* W/A FW bug: seq_ctl is wrong upon failure / BAR frame */
		if (ieee80211_is_back_req(hdr->frame_control))
			seq_ctl = 0;
		else if (status != TX_STATUS_SUCCESS)
			seq_ctl = le16_to_cpu(hdr->seq_ctrl);

		if (unlikely(!seq_ctl)) {
			struct ieee80211_hdr *hdr = (void *)skb->data;

			/*
			 * If it is an NDP, we can't update next_reclaim since
			 * its sequence control is 0. Note that for that same
			 * reason, NDPs are never sent to A-MPDU'able queues
			 * so that we can never have more than one freed frame
			 * for a single Tx resonse (see WARN_ON below).
			 */
			if (ieee80211_is_qos_nullfunc(hdr->frame_control))
				is_ndp = true;
		}

		/*
		 * TODO: this is not accurate if we are freeing more than one
		 * packet.
		 */
		info->status.tx_time =
			le16_to_cpu(tx_resp->wireless_media_time);
		BUILD_BUG_ON(ARRAY_SIZE(info->status.status_driver_data) < 1);
		lq_color = TX_RES_RATE_TABLE_COL_GET(tx_resp->tlc_info);
		info->status.status_driver_data[0] =
			RS_DRV_DATA_PACK(lq_color, tx_resp->reduced_tpc);

		ieee80211_tx_status(mvm->hw, skb);
	}

	/* This is an aggregation queue or might become one, so we use
	 * the ssn since: ssn = wifi seq_num % 256.
	 * The seq_ctl is the sequence control of the packet to which
	 * this Tx response relates. But if there is a hole in the
	 * bitmap of the BA we received, this Tx response may allow to
	 * reclaim the hole and all the subsequent packets that were
	 * already acked. In that case, seq_ctl != ssn, and the next
	 * packet to be reclaimed will be ssn and not seq_ctl. In that
	 * case, several packets will be reclaimed even if
	 * frame_count = 1.
	 *
	 * The ssn is the index (% 256) of the latest packet that has
	 * treated (acked / dropped) + 1.
	 */
	next_reclaimed = ssn;

	IWL_DEBUG_TX_REPLY(mvm,
			   "TXQ %d status %s (0x%08x)\n",
			   txq_id, iwl_mvm_get_tx_fail_reason(status), status);

	IWL_DEBUG_TX_REPLY(mvm,
			   "\t\t\t\tinitial_rate 0x%x retries %d, idx=%d ssn=%d next_reclaimed=0x%x seq_ctl=0x%x\n",
			   le32_to_cpu(tx_resp->initial_rate),
			   tx_resp->failure_frame, SEQ_TO_INDEX(sequence),
			   ssn, next_reclaimed, seq_ctl);

	rcu_read_lock();

	sta = rcu_dereference(mvm->fw_id_to_mac_id[sta_id]);
	/*
	 * sta can't be NULL otherwise it'd mean that the sta has been freed in
	 * the firmware while we still have packets for it in the Tx queues.
	 */
	if (WARN_ON_ONCE(!sta))
		goto out;

	if (!IS_ERR(sta)) {
		struct iwl_mvm_sta *mvmsta = iwl_mvm_sta_from_mac80211(sta);

		iwl_mvm_tx_airtime(mvm, mvmsta,
				   le16_to_cpu(tx_resp->wireless_media_time));

		if ((status & TX_STATUS_MSK) != TX_STATUS_SUCCESS &&
		    mvmsta->sta_state < IEEE80211_STA_AUTHORIZED)
			iwl_mvm_toggle_tx_ant(mvm, &mvmsta->tx_ant);

		if (sta->wme && tid != IWL_MGMT_TID) {
			struct iwl_mvm_tid_data *tid_data =
				&mvmsta->tid_data[tid];
			bool send_eosp_ndp = false;

			spin_lock_bh(&mvmsta->lock);

			if (!is_ndp) {
				tid_data->next_reclaimed = next_reclaimed;
				IWL_DEBUG_TX_REPLY(mvm,
						   "Next reclaimed packet:%d\n",
						   next_reclaimed);
			} else {
				IWL_DEBUG_TX_REPLY(mvm,
						   "NDP - don't update next_reclaimed\n");
			}

			iwl_mvm_check_ratid_empty(mvm, sta, tid);

			if (mvmsta->sleep_tx_count) {
				mvmsta->sleep_tx_count--;
				if (mvmsta->sleep_tx_count &&
				    !iwl_mvm_tid_queued(mvm, tid_data)) {
					/*
					 * The number of frames in the queue
					 * dropped to 0 even if we sent less
					 * frames than we thought we had on the
					 * Tx queue.
					 * This means we had holes in the BA
					 * window that we just filled, ask
					 * mac80211 to send EOSP since the
					 * firmware won't know how to do that.
					 * Send NDP and the firmware will send
					 * EOSP notification that will trigger
					 * a call to ieee80211_sta_eosp().
					 */
					send_eosp_ndp = true;
				}
			}

			spin_unlock_bh(&mvmsta->lock);
			if (send_eosp_ndp) {
				iwl_mvm_sta_modify_sleep_tx_count(mvm, sta,
					IEEE80211_FRAME_RELEASE_UAPSD,
					1, tid, false, false);
				mvmsta->sleep_tx_count = 0;
				ieee80211_send_eosp_nullfunc(sta, tid);
			}
		}
		switch (status & TX_STATUS_MSK) {
		case TX_STATUS_SUCCESS:
		case TX_STATUS_DIRECT_DONE:
			info->flags |= IEEE80211_TX_STAT_ACK;
			break;
		case TX_STATUS_FAIL_FIFO_FLUSHED:
		case TX_STATUS_FAIL_DRAIN_FLOW:
			flushed = true;
			break;
		case TX_STATUS_FAIL_DEST_PS:
			/* the FW should have stopped the queue and not
			 * return this status
			 */
			IWL_ERR_LIMIT(mvm,
				      "FW reported TX filtered, status=0x%x, FC=0x%x\n",
				      status, le16_to_cpu(hdr->frame_control));
			info->flags |= IEEE80211_TX_STAT_TX_FILTERED;
			break;
		default:
			break;
		}

		if ((status & TX_STATUS_MSK) != TX_STATUS_SUCCESS &&
		    ieee80211_is_mgmt(hdr->frame_control))
			iwl_mvm_toggle_tx_ant(mvm, &mvm->mgmt_last_antenna_idx);

		/*
		 * If we are freeing multiple frames, mark all the frames
		 * but the first one as acked, since they were acknowledged
		 * before
		 * */
		if (skb_freed > 1)
			info->flags |= IEEE80211_TX_STAT_ACK;

		iwl_mvm_tx_status_check_trigger(mvm, status, hdr->frame_control);

		info->status.rates[0].count = tx_resp->failure_frame + 1;

		iwl_mvm_hwrate_to_tx_status(mvm->fw,
					    le32_to_cpu(tx_resp->initial_rate),
					    info);

		/* Don't assign the converted initial_rate, because driver
		 * TLC uses this and doesn't support the new FW rate
		 */
		info->status.status_driver_data[1] =
			(void *)(uintptr_t)le32_to_cpu(tx_resp->initial_rate);

		/* Single frame failure in an AMPDU queue => send BAR */
		if (info->flags & IEEE80211_TX_CTL_AMPDU &&
		    !(info->flags & IEEE80211_TX_STAT_ACK) &&
		    !(info->flags & IEEE80211_TX_STAT_TX_FILTERED) && !flushed)
			info->flags |= IEEE80211_TX_STAT_AMPDU_NO_BACK;
		info->flags &= ~IEEE80211_TX_CTL_AMPDU;

		/* W/A FW bug: seq_ctl is wrong upon failure / BAR frame */
		if (ieee80211_is_back_req(hdr->frame_control))
			seq_ctl = 0;
		else if (status != TX_STATUS_SUCCESS)
			seq_ctl = le16_to_cpu(hdr->seq_ctrl);

		if (unlikely(!seq_ctl)) {
			struct ieee80211_hdr *hdr = (void *)skb->data;

			/*
			 * If it is an NDP, we can't update next_reclaim since
			 * its sequence control is 0. Note that for that same
			 * reason, NDPs are never sent to A-MPDU'able queues
			 * so that we can never have more than one freed frame
			 * for a single Tx resonse (see WARN_ON below).
			 */
			if (ieee80211_is_qos_nullfunc(hdr->frame_control))
				is_ndp = true;
		}
		if (freed == 1) {
			info->flags |= IEEE80211_TX_STAT_AMPDU;
			memcpy(&info->status, &tx_info->status,
			       sizeof(tx_info->status));
			iwl_mvm_hwrate_to_tx_status(mvm->fw, rate, info);
		}
	}

	spin_unlock_bh(&mvmsta->lock);

	/* We got a BA notif with 0 acked or scd_ssn didn't progress which is
	 * possible (i.e. first MPDU in the aggregation wasn't acked)
	 * Still it's important to update RS about sent vs. acked.
	 */
	if (!is_flush && skb_queue_empty(&reclaimed_skbs)) {
		struct ieee80211_chanctx_conf *chanctx_conf = NULL;

		if (mvmsta->vif)
			chanctx_conf =
				rcu_dereference(mvmsta->vif->chanctx_conf);

		if (WARN_ON_ONCE(!chanctx_conf))
			goto out;

		tx_info->band = chanctx_conf->def.chan->band;
		iwl_mvm_hwrate_to_tx_status(mvm->fw, rate, tx_info);

		if (!iwl_mvm_has_tlc_offload(mvm)) {
			IWL_DEBUG_TX_REPLY(mvm,
					   "No reclaim. Update rs directly\n");
			iwl_mvm_rs_tx_status(mvm, sta, tid, tx_info, false);
		}
	}

out:
	rcu_read_unlock();

	while (!skb_queue_empty(&reclaimed_skbs)) {
		skb = __skb_dequeue(&reclaimed_skbs);
		ieee80211_tx_status(mvm->hw, skb);
	}
}

void iwl_mvm_rx_ba_notif(struct iwl_mvm *mvm, struct iwl_rx_cmd_buffer *rxb)
{
	struct iwl_rx_packet *pkt = rxb_addr(rxb);
	unsigned int pkt_len = iwl_rx_packet_payload_len(pkt);
	int sta_id, tid, txq, index;
	struct ieee80211_tx_info ba_info = {};
	struct iwl_mvm_ba_notif *ba_notif;
	struct iwl_mvm_tid_data *tid_data;
	struct iwl_mvm_sta *mvmsta;

	ba_info.flags = IEEE80211_TX_STAT_AMPDU;

	if (iwl_mvm_has_new_tx_api(mvm)) {
		struct iwl_mvm_compressed_ba_notif *ba_res =
			(void *)pkt->data;
		u8 lq_color = TX_RES_RATE_TABLE_COL_GET(ba_res->tlc_rate_info);
		u16 tfd_cnt;
		int i;

		if (unlikely(sizeof(*ba_res) > pkt_len))
			return;

		sta_id = ba_res->sta_id;
		ba_info.status.ampdu_ack_len = (u8)le16_to_cpu(ba_res->done);
		ba_info.status.ampdu_len = (u8)le16_to_cpu(ba_res->txed);
		ba_info.status.tx_time =
			(u16)le32_to_cpu(ba_res->wireless_time);
		ba_info.status.status_driver_data[0] =
			(void *)(uintptr_t)ba_res->reduced_txp;

		tfd_cnt = le16_to_cpu(ba_res->tfd_cnt);
		if (!tfd_cnt || struct_size(ba_res, tfd, tfd_cnt) > pkt_len)
			return;

		rcu_read_lock();

		mvmsta = iwl_mvm_sta_from_staid_rcu(mvm, sta_id);
		/*
		 * It's possible to get a BA response after invalidating the rcu
		 * (rcu is invalidated in order to prevent new Tx from being
		 * sent, but there may be some frames already in-flight).
		 * In this case we just want to reclaim, and could skip all the
		 * sta-dependent stuff since it's in the middle of being removed
		 * anyways.
		 */

		/* Free per TID */
		for (i = 0; i < tfd_cnt; i++) {
			struct iwl_mvm_compressed_ba_tfd *ba_tfd =
				&ba_res->tfd[i];

			tid = ba_tfd->tid;
			if (tid == IWL_MGMT_TID)
				tid = IWL_MAX_TID_COUNT;

			if (mvmsta)
				mvmsta->tid_data[i].lq_color = lq_color;

			iwl_mvm_tx_reclaim(mvm, sta_id, tid,
					   (int)(le16_to_cpu(ba_tfd->q_num)),
					   le16_to_cpu(ba_tfd->tfd_index),
					   &ba_info,
					   le32_to_cpu(ba_res->tx_rate), false);
		}

		if (mvmsta)
			iwl_mvm_tx_airtime(mvm, mvmsta,
					   le32_to_cpu(ba_res->wireless_time));
		rcu_read_unlock();

		IWL_DEBUG_TX_REPLY(mvm,
				   "BA_NOTIFICATION Received from sta_id = %d, flags %x, sent:%d, acked:%d\n",
				   sta_id, le32_to_cpu(ba_res->flags),
				   le16_to_cpu(ba_res->txed),
				   le16_to_cpu(ba_res->done));
		return;
	}

	ba_notif = (void *)pkt->data;
	sta_id = ba_notif->sta_id;
	tid = ba_notif->tid;
	/* "flow" corresponds to Tx queue */
	txq = le16_to_cpu(ba_notif->scd_flow);
	/* "ssn" is start of block-ack Tx window, corresponds to index
	 * (in Tx queue's circular buffer) of first TFD/frame in window */
	index = le16_to_cpu(ba_notif->scd_ssn);

	rcu_read_lock();
	mvmsta = iwl_mvm_sta_from_staid_rcu(mvm, sta_id);
	if (WARN_ON_ONCE(!mvmsta)) {
		rcu_read_unlock();
		return;
	}

	tid_data = &mvmsta->tid_data[tid];

	ba_info.status.ampdu_ack_len = ba_notif->txed_2_done;
	ba_info.status.ampdu_len = ba_notif->txed;
	ba_info.status.tx_time = tid_data->tx_time;
	ba_info.status.status_driver_data[0] =
		(void *)(uintptr_t)ba_notif->reduced_txp;

	rcu_read_unlock();

	iwl_mvm_tx_reclaim(mvm, sta_id, tid, txq, index, &ba_info,
			   tid_data->rate_n_flags, false);

	IWL_DEBUG_TX_REPLY(mvm,
			   "BA_NOTIFICATION Received from %pM, sta_id = %d\n",
			   ba_notif->sta_addr, ba_notif->sta_id);

	IWL_DEBUG_TX_REPLY(mvm,
			   "TID = %d, SeqCtl = %d, bitmap = 0x%llx, scd_flow = %d, scd_ssn = %d sent:%d, acked:%d\n",
			   ba_notif->tid, le16_to_cpu(ba_notif->seq_ctl),
			   le64_to_cpu(ba_notif->bitmap), txq, index,
			   ba_notif->txed, ba_notif->txed_2_done);

	IWL_DEBUG_TX_REPLY(mvm, "reduced txp from ba notif %d\n",
			   ba_notif->reduced_txp);
}

/*
 * Note that there are transports that buffer frames before they reach
 * the firmware. This means that after flush_tx_path is called, the
 * queue might not be empty. The race-free way to handle this is to:
 * 1) set the station as draining
 * 2) flush the Tx path
 * 3) wait for the transport queues to be empty
 */
int iwl_mvm_flush_tx_path(struct iwl_mvm *mvm, u32 tfd_msk)
{
	int ret;
	struct iwl_tx_path_flush_cmd_v1 flush_cmd = {
		.queues_ctl = cpu_to_le32(tfd_msk),
		.flush_ctl = cpu_to_le16(DUMP_TX_FIFO_FLUSH),
	};

	WARN_ON(iwl_mvm_has_new_tx_api(mvm));
	ret = iwl_mvm_send_cmd_pdu(mvm, TXPATH_FLUSH, 0,
				   sizeof(flush_cmd), &flush_cmd);
	if (ret)
		IWL_ERR(mvm, "Failed to send flush command (%d)\n", ret);
	return ret;
}

int iwl_mvm_flush_sta_tids(struct iwl_mvm *mvm, u32 sta_id, u16 tids)
{
	int ret;
	struct iwl_tx_path_flush_cmd_rsp *rsp;
	struct iwl_tx_path_flush_cmd flush_cmd = {
		.sta_id = cpu_to_le32(sta_id),
		.tid_mask = cpu_to_le16(tids),
	};

	struct iwl_host_cmd cmd = {
		.id = TXPATH_FLUSH,
		.len = { sizeof(flush_cmd), },
		.data = { &flush_cmd, },
	};

	WARN_ON(!iwl_mvm_has_new_tx_api(mvm));

	if (iwl_fw_lookup_notif_ver(mvm->fw, LONG_GROUP, TXPATH_FLUSH, 0) > 0)
		cmd.flags |= CMD_WANT_SKB;

	IWL_DEBUG_TX_QUEUES(mvm, "flush for sta id %d tid mask 0x%x\n",
			    sta_id, tids);

	ret = iwl_mvm_send_cmd(mvm, &cmd);

	if (ret) {
		IWL_ERR(mvm, "Failed to send flush command (%d)\n", ret);
		return ret;
	}

	if (cmd.flags & CMD_WANT_SKB) {
		int i;
		int num_flushed_queues;

		if (WARN_ON_ONCE(iwl_rx_packet_payload_len(cmd.resp_pkt) != sizeof(*rsp))) {
			ret = -EIO;
			goto free_rsp;
		}

		rsp = (void *)cmd.resp_pkt->data;

		if (WARN_ONCE(le16_to_cpu(rsp->sta_id) != sta_id,
			      "sta_id %d != rsp_sta_id %d",
			      sta_id, le16_to_cpu(rsp->sta_id))) {
			ret = -EIO;
			goto free_rsp;
		}

		num_flushed_queues = le16_to_cpu(rsp->num_flushed_queues);
		if (WARN_ONCE(num_flushed_queues > IWL_TX_FLUSH_QUEUE_RSP,
			      "num_flushed_queues %d", num_flushed_queues)) {
			ret = -EIO;
			goto free_rsp;
		}

		for (i = 0; i < num_flushed_queues; i++) {
			struct ieee80211_tx_info tx_info = {};
			struct iwl_flush_queue_info *queue_info = &rsp->queues[i];
			int tid = le16_to_cpu(queue_info->tid);
			int read_before = le16_to_cpu(queue_info->read_before_flush);
			int read_after = le16_to_cpu(queue_info->read_after_flush);
			int queue_num = le16_to_cpu(queue_info->queue_num);

			if (tid == IWL_MGMT_TID)
				tid = IWL_MAX_TID_COUNT;

			IWL_DEBUG_TX_QUEUES(mvm,
					    "tid %d queue_id %d read-before %d read-after %d\n",
					    tid, queue_num, read_before, read_after);

			iwl_mvm_tx_reclaim(mvm, sta_id, tid, queue_num, read_after,
					   &tx_info, 0, true);
		}
free_rsp:
		iwl_free_resp(&cmd);
	}
	return ret;
}
	if (!is_flush && skb_queue_empty(&reclaimed_skbs)) {
		struct ieee80211_chanctx_conf *chanctx_conf = NULL;

		if (mvmsta->vif)
			chanctx_conf =
				rcu_dereference(mvmsta->vif->chanctx_conf);

		if (WARN_ON_ONCE(!chanctx_conf))
			goto out;

		tx_info->band = chanctx_conf->def.chan->band;
		iwl_mvm_hwrate_to_tx_status(mvm->fw, rate, tx_info);

		if (!iwl_mvm_has_tlc_offload(mvm)) {
			IWL_DEBUG_TX_REPLY(mvm,
					   "No reclaim. Update rs directly\n");
			iwl_mvm_rs_tx_status(mvm, sta, tid, tx_info, false);
		}