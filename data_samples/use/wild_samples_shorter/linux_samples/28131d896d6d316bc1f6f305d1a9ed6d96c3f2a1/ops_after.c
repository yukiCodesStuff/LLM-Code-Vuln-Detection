
#define DRV_DESCRIPTION	"The new Intel(R) wireless AGN driver for Linux"
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_LICENSE("GPL");

static const struct iwl_op_mode_ops iwl_mvm_ops;
static const struct iwl_op_mode_ops iwl_mvm_ops_mq;
		       iwl_mvm_probe_resp_data_notif,
		       RX_HANDLER_ASYNC_LOCKED,
		       struct iwl_probe_resp_data_notif),
	RX_HANDLER_GRP(MAC_CONF_GROUP, CHANNEL_SWITCH_START_NOTIF,
		       iwl_mvm_channel_switch_start_notif,
		       RX_HANDLER_SYNC, struct iwl_channel_switch_start_notif),
	RX_HANDLER_GRP(DATA_PATH_GROUP, MONITOR_NOTIF,
		       iwl_mvm_rx_monitor_notif, RX_HANDLER_ASYNC_LOCKED,
		       struct iwl_datapath_monitor_notif),

	HCMD_NAME(CHANNEL_SWITCH_TIME_EVENT_CMD),
	HCMD_NAME(SESSION_PROTECTION_CMD),
	HCMD_NAME(SESSION_PROTECTION_NOTIF),
	HCMD_NAME(CHANNEL_SWITCH_START_NOTIF),
};

/* Please keep this array *SORTED* by hex value.
 * Access is done through binary search
	HCMD_NAME(CMD_DTS_MEASUREMENT_TRIGGER_WIDE),
	HCMD_NAME(CTDP_CONFIG_CMD),
	HCMD_NAME(TEMP_REPORTING_THRESHOLDS_CMD),
	HCMD_NAME(PER_CHAIN_LIMIT_OFFSET_CMD),
	HCMD_NAME(CT_KILL_NOTIFICATION),
	HCMD_NAME(DTS_MEASUREMENT_NOTIF_WIDE),
};

	return 0;
}

struct iwl_mvm_frob_txf_data {
	u8 *buf;
	size_t buflen;
};

static void iwl_mvm_frob_txf_key_iter(struct ieee80211_hw *hw,
				      struct ieee80211_vif *vif,
				      struct ieee80211_sta *sta,
				      struct ieee80211_key_conf *key,
				      void *data)
{
	struct iwl_mvm_frob_txf_data *txf = data;
	u8 keylen, match, matchend;
	u8 *keydata;
	size_t i;

	switch (key->cipher) {
	case WLAN_CIPHER_SUITE_CCMP:
		keydata = key->key;
		keylen = key->keylen;
		break;
	case WLAN_CIPHER_SUITE_WEP40:
	case WLAN_CIPHER_SUITE_WEP104:
	case WLAN_CIPHER_SUITE_TKIP:
		/*
		 * WEP has short keys which might show up in the payload,
		 * and then you can deduce the key, so in this case just
		 * remove all FIFO data.
		 * For TKIP, we don't know the phase 2 keys here, so same.
		 */
		memset(txf->buf, 0xBB, txf->buflen);
		return;
	default:
		return;
	}

	/* scan for key material and clear it out */
	match = 0;
	for (i = 0; i < txf->buflen; i++) {
		if (txf->buf[i] != keydata[match]) {
			match = 0;
			continue;
		}
		match++;
		if (match == keylen) {
			memset(txf->buf + i - keylen, 0xAA, keylen);
			match = 0;
		}
	}

	/* we're dealing with a FIFO, so check wrapped around data */
	matchend = match;
	for (i = 0; match && i < keylen - match; i++) {
		if (txf->buf[i] != keydata[match])
			break;
		match++;
		if (match == keylen) {
			memset(txf->buf, 0xAA, i + 1);
			memset(txf->buf + txf->buflen - matchend, 0xAA,
			       matchend);
			break;
		}
	}
}

static void iwl_mvm_frob_txf(void *ctx, void *buf, size_t buflen)
{
	struct iwl_mvm_frob_txf_data txf = {
		.buf = buf,
		.buflen = buflen,
	};
	struct iwl_mvm *mvm = ctx;

	/* embedded key material exists only on old API */
	if (iwl_mvm_has_new_tx_api(mvm))
		return;

	rcu_read_lock();
	ieee80211_iter_keys_rcu(mvm->hw, NULL, iwl_mvm_frob_txf_key_iter, &txf);
	rcu_read_unlock();
}

static void iwl_mvm_frob_hcmd(void *ctx, void *hcmd, size_t len)
{
	/* we only use wide headers for commands */
	struct iwl_cmd_header_wide *hdr = hcmd;
	unsigned int frob_start = sizeof(*hdr), frob_end = 0;

	if (len < sizeof(hdr))
		return;

	/* all the commands we care about are in LONG_GROUP */
	if (hdr->group_id != LONG_GROUP)
		return;

	switch (hdr->cmd) {
	case WEP_KEY:
	case WOWLAN_TKIP_PARAM:
	case WOWLAN_KEK_KCK_MATERIAL:
	case ADD_STA_KEY:
		/*
		 * blank out everything here, easier than dealing
		 * with the various versions of the command
		 */
		frob_end = INT_MAX;
		break;
	case MGMT_MCAST_KEY:
		frob_start = offsetof(struct iwl_mvm_mgmt_mcast_key_cmd, igtk);
		BUILD_BUG_ON(offsetof(struct iwl_mvm_mgmt_mcast_key_cmd, igtk) !=
			     offsetof(struct iwl_mvm_mgmt_mcast_key_cmd_v1, igtk));

		frob_end = offsetofend(struct iwl_mvm_mgmt_mcast_key_cmd, igtk);
		BUILD_BUG_ON(offsetof(struct iwl_mvm_mgmt_mcast_key_cmd, igtk) <
			     offsetof(struct iwl_mvm_mgmt_mcast_key_cmd_v1, igtk));
		break;
	}

	if (frob_start >= frob_end)
		return;

	if (frob_end > len)
		frob_end = len;

	memset((u8 *)hcmd + frob_start, 0xAA, frob_end - frob_start);
}

static void iwl_mvm_frob_mem(void *ctx, u32 mem_addr, void *mem, size_t buflen)
{
	const struct iwl_dump_exclude *excl;
	struct iwl_mvm *mvm = ctx;
	int i;

	switch (mvm->fwrt.cur_fw_img) {
	case IWL_UCODE_INIT:
	default:
		/* not relevant */
		return;
	case IWL_UCODE_REGULAR:
	case IWL_UCODE_REGULAR_USNIFFER:
		excl = mvm->fw->dump_excl;
		break;
	case IWL_UCODE_WOWLAN:
		excl = mvm->fw->dump_excl_wowlan;
		break;
	}

	BUILD_BUG_ON(sizeof(mvm->fw->dump_excl) !=
		     sizeof(mvm->fw->dump_excl_wowlan));

	for (i = 0; i < ARRAY_SIZE(mvm->fw->dump_excl); i++) {
		u32 start, end;

		if (!excl[i].addr || !excl[i].size)
			continue;

		start = excl[i].addr;
		end = start + excl[i].size;

		if (end <= mem_addr || start >= mem_addr + buflen)
			continue;

		if (start < mem_addr)
			start = mem_addr;

		if (end > mem_addr + buflen)
			end = mem_addr + buflen;

		memset((u8 *)mem + start - mem_addr, 0xAA, end - start);
	}
}

static const struct iwl_dump_sanitize_ops iwl_mvm_sanitize_ops = {
	.frob_txf = iwl_mvm_frob_txf,
	.frob_hcmd = iwl_mvm_frob_hcmd,
	.frob_mem = iwl_mvm_frob_mem,
};

static struct iwl_op_mode *
iwl_op_mode_mvm_start(struct iwl_trans *trans, const struct iwl_cfg *cfg,
		      const struct iwl_fw *fw, struct dentry *dbgfs_dir)
{
	mvm->hw = hw;

	iwl_fw_runtime_init(&mvm->fwrt, trans, fw, &iwl_mvm_fwrt_ops, mvm,
			    &iwl_mvm_sanitize_ops, mvm, dbgfs_dir);

	iwl_mvm_get_acpi_tables(mvm);

	mvm->init_status = 0;
	mvm->cmd_ver.range_resp =
		iwl_fw_lookup_notif_ver(mvm->fw, LOCATION_GROUP,
					TOF_RANGE_RESPONSE_NOTIF, 5);
	/* we only support up to version 9 */
	if (WARN_ON_ONCE(mvm->cmd_ver.range_resp > 9))
		goto out_free;

	/*
	 * Populate the state variables that the transport layer needs