struct ieee802_11_elems {
	const u8 *ie_start;
	size_t total_len;
	u32 crc;

	/* pointers to IEs */
	const struct ieee80211_tdls_lnkie *lnk_id;
	const struct ieee80211_ch_switch_timing *ch_sw_timing;
	const u8 *ext_capab;
	const u8 *ssid;
	const u8 *supp_rates;
	const u8 *ds_params;
	const struct ieee80211_tim_ie *tim;
	const u8 *rsn;
	const u8 *rsnx;
	const u8 *erp_info;
	const u8 *ext_supp_rates;
	const u8 *wmm_info;
	const u8 *wmm_param;
	const struct ieee80211_ht_cap *ht_cap_elem;
	const struct ieee80211_ht_operation *ht_operation;
	const struct ieee80211_vht_cap *vht_cap_elem;
	const struct ieee80211_vht_operation *vht_operation;
	const struct ieee80211_meshconf_ie *mesh_config;
	const u8 *he_cap;
	const struct ieee80211_he_operation *he_operation;
	const struct ieee80211_he_spr *he_spr;
	const struct ieee80211_mu_edca_param_set *mu_edca_param_set;
	const struct ieee80211_he_6ghz_capa *he_6ghz_capa;
	const struct ieee80211_tx_pwr_env *tx_pwr_env[IEEE80211_TPE_MAX_IE_COUNT];
	const u8 *uora_element;
	const u8 *mesh_id;
	const u8 *peering;
	const __le16 *awake_window;
	const u8 *preq;
	const u8 *prep;
	const u8 *perr;
	const struct ieee80211_rann_ie *rann;
	const struct ieee80211_channel_sw_ie *ch_switch_ie;
	const struct ieee80211_ext_chansw_ie *ext_chansw_ie;
	const struct ieee80211_wide_bw_chansw_ie *wide_bw_chansw_ie;
	const u8 *max_channel_switch_time;
	const u8 *country_elem;
	const u8 *pwr_constr_elem;
	const u8 *cisco_dtpc_elem;
	const struct ieee80211_timeout_interval_ie *timeout_int;
	const u8 *opmode_notif;
	const struct ieee80211_sec_chan_offs_ie *sec_chan_offs;
	struct ieee80211_mesh_chansw_params_ie *mesh_chansw_params_ie;
	const struct ieee80211_bss_max_idle_period_ie *max_idle_period_ie;
	const struct ieee80211_multiple_bssid_configuration *mbssid_config_ie;
	const struct ieee80211_bssid_index *bssid_index;
	u8 max_bssid_indicator;
	u8 dtim_count;
	u8 dtim_period;
	const struct ieee80211_addba_ext_ie *addba_ext_ie;
	const struct ieee80211_s1g_cap *s1g_capab;
	const struct ieee80211_s1g_oper_ie *s1g_oper;
	const struct ieee80211_s1g_bcn_compat_ie *s1g_bcn_compat;
	const struct ieee80211_aid_response_ie *aid_resp;
	const struct ieee80211_eht_cap_elem *eht_cap;
	const struct ieee80211_eht_operation *eht_operation;
	const struct ieee80211_multi_link_elem *multi_link;

	/* length of them, respectively */
	u8 ext_capab_len;
	u8 ssid_len;
	u8 supp_rates_len;
	u8 tim_len;
	u8 rsn_len;
	u8 rsnx_len;
	u8 ext_supp_rates_len;
	u8 wmm_info_len;
	u8 wmm_param_len;
	u8 he_cap_len;
	u8 mesh_id_len;
	u8 peering_len;
	u8 preq_len;
	u8 prep_len;
	u8 perr_len;
	u8 country_elem_len;
	u8 bssid_index_len;
	u8 tx_pwr_env_len[IEEE80211_TPE_MAX_IE_COUNT];
	u8 tx_pwr_env_num;
	u8 eht_cap_len;

	/* whether a parse error occurred while retrieving these elements */
	bool parse_error;

	/*
	 * scratch buffer that can be used for various element parsing related
	 * tasks, e.g., element de-fragmentation etc.
	 */
	size_t scratch_len;
	u8 *scratch_pos;
	u8 scratch[];
};

static inline struct ieee80211_local *hw_to_local(
	struct ieee80211_hw *hw)
{
	return container_of(hw, struct ieee80211_local, hw);
}