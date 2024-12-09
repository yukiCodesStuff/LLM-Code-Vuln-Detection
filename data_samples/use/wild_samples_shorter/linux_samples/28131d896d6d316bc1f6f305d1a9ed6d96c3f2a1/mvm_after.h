 *	see enum &iwl_mvm_low_latency_cause for causes.
 * @low_latency_actual: boolean, indicates low latency is set,
 *	as a result from low_latency bit flags and takes force into account.
 * @authorized: indicates the AP station was set to authorized
 * @ps_disabled: indicates that this interface requires PS to be disabled
 * @queue_params: QoS params for this MAC
 * @bcast_sta: station used for broadcast packets. Used by the following
 *  vifs: P2P_DEVICE, GO and AP.
	bool monitor_active;
	u8 low_latency: 6;
	u8 low_latency_actual: 1;
	u8 authorized:1;
	bool ps_disabled;
	struct iwl_mvm_vif_bf_data bf_data;

	struct {
int iwl_run_init_mvm_ucode(struct iwl_mvm *mvm);

/* Utils */
int iwl_mvm_legacy_hw_idx_to_mac80211_idx(u32 rate_n_flags,
					  enum nl80211_band band);
int iwl_mvm_legacy_rate_to_mac80211_idx(u32 rate_n_flags,
					enum nl80211_band band);
void iwl_mvm_hwrate_to_tx_rate(u32 rate_n_flags,
			       enum nl80211_band band,
			       struct ieee80211_tx_rate *r);
void iwl_mvm_hwrate_to_tx_rate_v1(u32 rate_n_flags,
				  enum nl80211_band band,
				  struct ieee80211_tx_rate *r);
u8 iwl_mvm_mac80211_idx_to_hwrate(const struct iwl_fw *fw, int rate_idx);
u8 iwl_mvm_mac80211_ac_to_ucode_ac(enum ieee80211_ac_numbers ac);

static inline void iwl_mvm_dump_nic_error_log(struct iwl_mvm *mvm)
{
				     void *data, int len);
u8 iwl_mvm_mac_ctxt_get_lowest_rate(struct ieee80211_tx_info *info,
				    struct ieee80211_vif *vif);
u16 iwl_mvm_mac_ctxt_get_beacon_flags(const struct iwl_fw *fw,
				      u8 rate_idx);
void iwl_mvm_mac_ctxt_set_tim(struct iwl_mvm *mvm,
			      __le32 *tim_index, __le32 *tim_size,
			      u8 *beacon, u32 frame_size);
void iwl_mvm_rx_beacon_notif(struct iwl_mvm *mvm,
				   struct iwl_rx_cmd_buffer *rxb);
void iwl_mvm_rx_missed_vap_notif(struct iwl_mvm *mvm,
				 struct iwl_rx_cmd_buffer *rxb);
void iwl_mvm_channel_switch_start_notif(struct iwl_mvm *mvm,
					struct iwl_rx_cmd_buffer *rxb);
/* Bindings */
int iwl_mvm_binding_add_vif(struct iwl_mvm *mvm, struct ieee80211_vif *vif);
int iwl_mvm_binding_remove_vif(struct iwl_mvm *mvm, struct ieee80211_vif *vif);

/* rate scaling */
int iwl_mvm_send_lq_cmd(struct iwl_mvm *mvm, struct iwl_lq_cmd *lq);
void iwl_mvm_update_frame_stats(struct iwl_mvm *mvm, u32 rate, bool agg);
int rs_pretty_print_rate_v1(char *buf, int bufsz, const u32 rate);
void rs_update_last_rssi(struct iwl_mvm *mvm,
			 struct iwl_mvm_sta *mvmsta,
			 struct ieee80211_rx_status *rx_status);
