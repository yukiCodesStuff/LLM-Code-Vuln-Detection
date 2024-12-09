	struct iwl_mvm_mc_iter_data *data = _data;
	struct iwl_mvm *mvm = data->mvm;
	struct iwl_mcast_filter_cmd *cmd = mvm->mcast_filter_cmd;
	struct iwl_host_cmd hcmd = {
		.id = MCAST_FILTER_CMD,
		.flags = CMD_ASYNC,
		.dataflags[0] = IWL_HCMD_DFL_NOCOPY,
	};
	int ret, len;

	/* if we don't have free ports, mcast frames will be dropped */
	if (WARN_ON_ONCE(data->port_id >= MAX_PORT_ID_NUM))
	memcpy(cmd->bssid, vif->bss_conf.bssid, ETH_ALEN);
	len = roundup(sizeof(*cmd) + cmd->count * ETH_ALEN, 4);

	hcmd.len[0] = len;
	hcmd.data[0] = cmd;

	ret = iwl_mvm_send_cmd(mvm, &hcmd);
	if (ret)
		IWL_ERR(mvm, "mcast filter cmd error. ret=%d\n", ret);
}

	if (!cmd)
		goto out;

	if (changed_flags & FIF_ALLMULTI)
		cmd->pass_all = !!(*total_flags & FIF_ALLMULTI);

	if (cmd->pass_all)
		cmd->count = 0;

	iwl_mvm_recalc_multicast(mvm);
out:
	mutex_unlock(&mvm->mutex);
	*total_flags = 0;
			 * queues, so we should never get a second deferred
			 * frame for the RA/TID.
			 */
			iwl_mvm_start_mac_queues(mvm, BIT(info->hw_queue));
			ieee80211_free_txskb(mvm->hw, skb);
		}
	}
	spin_unlock_bh(&mvm_sta->lock);
	return ret;
}

static void iwl_mvm_flush_no_vif(struct iwl_mvm *mvm, u32 queues, bool drop)
{
	if (drop) {
		if (iwl_mvm_has_new_tx_api(mvm))
			/* TODO new tx api */
			WARN_ONCE(1,
				  "Need to implement flush TX queue\n");
		else
			iwl_mvm_flush_tx_path(mvm,
				iwl_mvm_flushable_queues(mvm) & queues,
				0);
	} else {
		if (iwl_mvm_has_new_tx_api(mvm)) {
			struct ieee80211_sta *sta;
			int i;

			mutex_lock(&mvm->mutex);

			for (i = 0; i < ARRAY_SIZE(mvm->fw_id_to_mac_id); i++) {
				sta = rcu_dereference_protected(
						mvm->fw_id_to_mac_id[i],
						lockdep_is_held(&mvm->mutex));
				if (IS_ERR_OR_NULL(sta))
					continue;

				iwl_mvm_wait_sta_queues_empty(mvm,
						iwl_mvm_sta_from_mac80211(sta));
			}

			mutex_unlock(&mvm->mutex);
		} else {
			iwl_trans_wait_tx_queues_empty(mvm->trans,
						       queues);
		}
	}
}

static void iwl_mvm_mac_flush(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif, u32 queues, bool drop)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);
	int i;
	u32 msk = 0;

	if (!vif) {
		iwl_mvm_flush_no_vif(mvm, queues, drop);
		return;
	}

	if (vif->type != NL80211_IFTYPE_STATION)
		return;

	/* Make sure we're done with the deferred traffic before flushing */
	flush_work(&mvm->add_stream_wk);