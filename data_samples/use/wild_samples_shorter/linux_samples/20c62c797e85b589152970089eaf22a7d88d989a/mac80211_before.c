	struct iwl_mvm_mc_iter_data *data = _data;
	struct iwl_mvm *mvm = data->mvm;
	struct iwl_mcast_filter_cmd *cmd = mvm->mcast_filter_cmd;
	int ret, len;

	/* if we don't have free ports, mcast frames will be dropped */
	if (WARN_ON_ONCE(data->port_id >= MAX_PORT_ID_NUM))
	memcpy(cmd->bssid, vif->bss_conf.bssid, ETH_ALEN);
	len = roundup(sizeof(*cmd) + cmd->count * ETH_ALEN, 4);

	ret = iwl_mvm_send_cmd_pdu(mvm, MCAST_FILTER_CMD, CMD_ASYNC, len, cmd);
	if (ret)
		IWL_ERR(mvm, "mcast filter cmd error. ret=%d\n", ret);
}

	if (!cmd)
		goto out;

	iwl_mvm_recalc_multicast(mvm);
out:
	mutex_unlock(&mvm->mutex);
	*total_flags = 0;
			 * queues, so we should never get a second deferred
			 * frame for the RA/TID.
			 */
			iwl_mvm_start_mac_queues(mvm, info->hw_queue);
			ieee80211_free_txskb(mvm->hw, skb);
		}
	}
	spin_unlock_bh(&mvm_sta->lock);
	return ret;
}

static void iwl_mvm_mac_flush(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif, u32 queues, bool drop)
{
	struct iwl_mvm *mvm = IWL_MAC80211_GET_MVM(hw);
	int i;
	u32 msk = 0;

	if (!vif || vif->type != NL80211_IFTYPE_STATION)
		return;

	/* Make sure we're done with the deferred traffic before flushing */
	flush_work(&mvm->add_stream_wk);