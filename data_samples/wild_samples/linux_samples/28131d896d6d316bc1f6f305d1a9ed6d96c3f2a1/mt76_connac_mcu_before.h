enum {
	MCU_EXT_CMD_EFUSE_ACCESS = 0x01,
	MCU_EXT_CMD_RF_REG_ACCESS = 0x02,
	MCU_EXT_CMD_PM_STATE_CTRL = 0x07,
	MCU_EXT_CMD_CHANNEL_SWITCH = 0x08,
	MCU_EXT_CMD_SET_TX_POWER_CTRL = 0x11,
	MCU_EXT_CMD_FW_LOG_2_HOST = 0x13,
	MCU_EXT_CMD_EFUSE_BUFFER_MODE = 0x21,
	MCU_EXT_CMD_STA_REC_UPDATE = 0x25,
	MCU_EXT_CMD_BSS_INFO_UPDATE = 0x26,
	MCU_EXT_CMD_EDCA_UPDATE = 0x27,
	MCU_EXT_CMD_DEV_INFO_UPDATE = 0x2A,
	MCU_EXT_CMD_GET_TEMP = 0x2c,
	MCU_EXT_CMD_WTBL_UPDATE = 0x32,
	MCU_EXT_CMD_SET_RDD_CTRL = 0x3a,
	MCU_EXT_CMD_ATE_CTRL = 0x3d,
	MCU_EXT_CMD_PROTECT_CTRL = 0x3e,
	MCU_EXT_CMD_DBDC_CTRL = 0x45,
	MCU_EXT_CMD_MAC_INIT_CTRL = 0x46,
	MCU_EXT_CMD_RX_HDR_TRANS = 0x47,
	MCU_EXT_CMD_MUAR_UPDATE = 0x48,
	MCU_EXT_CMD_BCN_OFFLOAD = 0x49,
	MCU_EXT_CMD_SET_RX_PATH = 0x4e,
	MCU_EXT_CMD_TX_POWER_FEATURE_CTRL = 0x58,
	MCU_EXT_CMD_RXDCOC_CAL = 0x59,
	MCU_EXT_CMD_TXDPD_CAL = 0x60,
	MCU_EXT_CMD_SET_RDD_TH = 0x7c,
	MCU_EXT_CMD_SET_RDD_PATTERN = 0x7d,
};

enum {
	MCU_UNI_CMD_DEV_INFO_UPDATE = MCU_UNI_PREFIX | 0x01,
	MCU_UNI_CMD_BSS_INFO_UPDATE = MCU_UNI_PREFIX | 0x02,
	MCU_UNI_CMD_STA_REC_UPDATE = MCU_UNI_PREFIX | 0x03,
	MCU_UNI_CMD_SUSPEND = MCU_UNI_PREFIX | 0x05,
	MCU_UNI_CMD_OFFLOAD = MCU_UNI_PREFIX | 0x06,
	MCU_UNI_CMD_HIF_CTRL = MCU_UNI_PREFIX | 0x07,
};

enum {
	MCU_CMD_TARGET_ADDRESS_LEN_REQ = MCU_FW_PREFIX | 0x01,
	MCU_CMD_FW_START_REQ = MCU_FW_PREFIX | 0x02,
	MCU_CMD_INIT_ACCESS_REG = 0x3,
	MCU_CMD_NIC_POWER_CTRL = MCU_FW_PREFIX | 0x4,
	MCU_CMD_PATCH_START_REQ = MCU_FW_PREFIX | 0x05,
	MCU_CMD_PATCH_FINISH_REQ = MCU_FW_PREFIX | 0x07,
	MCU_CMD_PATCH_SEM_CONTROL = MCU_FW_PREFIX | 0x10,
	MCU_CMD_EXT_CID = 0xed,
	MCU_CMD_FW_SCATTER = MCU_FW_PREFIX | 0xee,
	MCU_CMD_RESTART_DL_REQ = MCU_FW_PREFIX | 0xef,
};

/* offload mcu commands */
enum {
	MCU_CMD_TEST_CTRL = MCU_CE_PREFIX | 0x01,
	MCU_CMD_START_HW_SCAN = MCU_CE_PREFIX | 0x03,
	MCU_CMD_SET_PS_PROFILE = MCU_CE_PREFIX | 0x05,
	MCU_CMD_SET_CHAN_DOMAIN = MCU_CE_PREFIX | 0x0f,
	MCU_CMD_SET_BSS_CONNECTED = MCU_CE_PREFIX | 0x16,
	MCU_CMD_SET_BSS_ABORT = MCU_CE_PREFIX | 0x17,
	MCU_CMD_CANCEL_HW_SCAN = MCU_CE_PREFIX | 0x1b,
	MCU_CMD_SET_ROC = MCU_CE_PREFIX | 0x1d,
	MCU_CMD_SET_P2P_OPPPS = MCU_CE_PREFIX | 0x33,
	MCU_CMD_SET_RATE_TX_POWER = MCU_CE_PREFIX | 0x5d,
	MCU_CMD_SCHED_SCAN_ENABLE = MCU_CE_PREFIX | 0x61,
	MCU_CMD_SCHED_SCAN_REQ = MCU_CE_PREFIX | 0x62,
	MCU_CMD_GET_NIC_CAPAB = MCU_CE_PREFIX | 0x8a,
	MCU_CMD_SET_MU_EDCA_PARMS = MCU_CE_PREFIX | 0xb0,
	MCU_CMD_REG_WRITE = MCU_CE_PREFIX | 0xc0,
	MCU_CMD_REG_READ = MCU_CE_PREFIX | MCU_QUERY_MASK | 0xc0,
	MCU_CMD_CHIP_CONFIG = MCU_CE_PREFIX | 0xca,
	MCU_CMD_FWLOG_2_HOST = MCU_CE_PREFIX | 0xc5,
	MCU_CMD_GET_WTBL = MCU_CE_PREFIX | 0xcd,
	MCU_CMD_GET_TXPWR = MCU_CE_PREFIX | 0xd0,
};

enum {
	PATCH_SEM_RELEASE,
	PATCH_SEM_GET
};

enum {
	UNI_BSS_INFO_BASIC = 0,
	UNI_BSS_INFO_RLM = 2,
	UNI_BSS_INFO_BSS_COLOR = 4,
	UNI_BSS_INFO_HE_BASIC = 5,
	UNI_BSS_INFO_BCN_CONTENT = 7,
	UNI_BSS_INFO_QBSS = 15,
	UNI_BSS_INFO_UAPSD = 19,
	UNI_BSS_INFO_PS = 21,
	UNI_BSS_INFO_BCNFT = 22,
};

enum {
	UNI_OFFLOAD_OFFLOAD_ARP,
	UNI_OFFLOAD_OFFLOAD_ND,
	UNI_OFFLOAD_OFFLOAD_GTK_REKEY,
	UNI_OFFLOAD_OFFLOAD_BMC_RPY_DETECT,
};

enum {
	MT_NIC_CAP_TX_RESOURCE,
	MT_NIC_CAP_TX_EFUSE_ADDR,
	MT_NIC_CAP_COEX,
	MT_NIC_CAP_SINGLE_SKU,
	MT_NIC_CAP_CSUM_OFFLOAD,
	MT_NIC_CAP_HW_VER,
	MT_NIC_CAP_SW_VER,
	MT_NIC_CAP_MAC_ADDR,
	MT_NIC_CAP_PHY,
	MT_NIC_CAP_MAC,
	MT_NIC_CAP_FRAME_BUF,
	MT_NIC_CAP_BEAM_FORM,
	MT_NIC_CAP_LOCATION,
	MT_NIC_CAP_MUMIMO,
	MT_NIC_CAP_BUFFER_MODE_INFO,
	MT_NIC_CAP_HW_ADIE_VERSION = 0x14,
	MT_NIC_CAP_ANTSWP = 0x16,
	MT_NIC_CAP_WFDMA_REALLOC,
	MT_NIC_CAP_6G,
};

#define UNI_WOW_DETECT_TYPE_MAGIC		BIT(0)
#define UNI_WOW_DETECT_TYPE_ANY			BIT(1)
#define UNI_WOW_DETECT_TYPE_DISCONNECT		BIT(2)
#define UNI_WOW_DETECT_TYPE_GTK_REKEY_FAIL	BIT(3)
#define UNI_WOW_DETECT_TYPE_BCN_LOST		BIT(4)
#define UNI_WOW_DETECT_TYPE_SCH_SCAN_HIT	BIT(5)
#define UNI_WOW_DETECT_TYPE_BITMAP		BIT(6)

enum {
	UNI_SUSPEND_MODE_SETTING,
	UNI_SUSPEND_WOW_CTRL,
	UNI_SUSPEND_WOW_GPIO_PARAM,
	UNI_SUSPEND_WOW_WAKEUP_PORT,
	UNI_SUSPEND_WOW_PATTERN,
};

enum {
	WOW_USB = 1,
	WOW_PCIE = 2,
	WOW_GPIO = 3,
};

struct mt76_connac_bss_basic_tlv {
	__le16 tag;
	__le16 len;
	u8 active;
	u8 omac_idx;
	u8 hw_bss_idx;
	u8 band_idx;
	__le32 conn_type;
	u8 conn_state;
	u8 wmm_idx;
	u8 bssid[ETH_ALEN];
	__le16 bmc_tx_wlan_idx;
	__le16 bcn_interval;
	u8 dtim_period;
	u8 phymode; /* bit(0): A
		     * bit(1): B
		     * bit(2): G
		     * bit(3): GN
		     * bit(4): AN
		     * bit(5): AC
		     * bit(6): AX2
		     * bit(7): AX5
		     * bit(8): AX6
		     */
	__le16 sta_idx;
	__le16 nonht_basic_phy;
	u8 phymode_ext; /* bit(0) AX_6G */
	u8 pad[1];
} __packed;

struct mt76_connac_bss_qos_tlv {
	__le16 tag;
	__le16 len;
	u8 qos;
	u8 pad[3];
} __packed;

struct mt76_connac_beacon_loss_event {
	u8 bss_idx;
	u8 reason;
	u8 pad[2];
} __packed;

struct mt76_connac_mcu_bss_event {
	u8 bss_idx;
	u8 is_absent;
	u8 free_quota;
	u8 pad;
} __packed;

struct mt76_connac_mcu_scan_ssid {
	__le32 ssid_len;
	u8 ssid[IEEE80211_MAX_SSID_LEN];
} __packed;

struct mt76_connac_mcu_scan_channel {
	u8 band; /* 1: 2.4GHz
		  * 2: 5.0GHz
		  * Others: Reserved
		  */
	u8 channel_num;
} __packed;

struct mt76_connac_mcu_scan_match {
	__le32 rssi_th;
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	u8 ssid_len;
	u8 rsv[3];
} __packed;

struct mt76_connac_hw_scan_req {
	u8 seq_num;
	u8 bss_idx;
	u8 scan_type; /* 0: PASSIVE SCAN
		       * 1: ACTIVE SCAN
		       */
	u8 ssid_type; /* BIT(0) wildcard SSID
		       * BIT(1) P2P wildcard SSID
		       * BIT(2) specified SSID + wildcard SSID
		       * BIT(2) + ssid_type_ext BIT(0) specified SSID only
		       */
	u8 ssids_num;
	u8 probe_req_num; /* Number of probe request for each SSID */
	u8 scan_func; /* BIT(0) Enable random MAC scan
		       * BIT(1) Disable DBDC scan type 1~3.
		       * BIT(2) Use DBDC scan type 3 (dedicated one RF to scan).
		       */
	u8 version; /* 0: Not support fields after ies.
		     * 1: Support fields after ies.
		     */
	struct mt76_connac_mcu_scan_ssid ssids[4];
	__le16 probe_delay_time;
	__le16 channel_dwell_time; /* channel Dwell interval */
	__le16 timeout_value;
	u8 channel_type; /* 0: Full channels
			  * 1: Only 2.4GHz channels
			  * 2: Only 5GHz channels
			  * 3: P2P social channel only (channel #1, #6 and #11)
			  * 4: Specified channels
			  * Others: Reserved
			  */
	u8 channels_num; /* valid when channel_type is 4 */
	/* valid when channels_num is set */
	struct mt76_connac_mcu_scan_channel channels[32];
	__le16 ies_len;
	u8 ies[MT76_CONNAC_SCAN_IE_LEN];
	/* following fields are valid if version > 0 */
	u8 ext_channels_num;
	u8 ext_ssids_num;
	__le16 channel_min_dwell_time;
	struct mt76_connac_mcu_scan_channel ext_channels[32];
	struct mt76_connac_mcu_scan_ssid ext_ssids[6];
	u8 bssid[ETH_ALEN];
	u8 random_mac[ETH_ALEN]; /* valid when BIT(1) in scan_func is set. */
	u8 pad[63];
	u8 ssid_type_ext;
} __packed;

#define MT76_CONNAC_SCAN_DONE_EVENT_MAX_CHANNEL_NUM		64

struct mt76_connac_hw_scan_done {
	u8 seq_num;
	u8 sparse_channel_num;
	struct mt76_connac_mcu_scan_channel sparse_channel;
	u8 complete_channel_num;
	u8 current_state;
	u8 version;
	u8 pad;
	__le32 beacon_scan_num;
	u8 pno_enabled;
	u8 pad2[3];
	u8 sparse_channel_valid_num;
	u8 pad3[3];
	u8 channel_num[MT76_CONNAC_SCAN_DONE_EVENT_MAX_CHANNEL_NUM];
	/* idle format for channel_idle_time
	 * 0: first bytes: idle time(ms) 2nd byte: dwell time(ms)
	 * 1: first bytes: idle time(8ms) 2nd byte: dwell time(8ms)
	 * 2: dwell time (16us)
	 */
	__le16 channel_idle_time[MT76_CONNAC_SCAN_DONE_EVENT_MAX_CHANNEL_NUM];
	/* beacon and probe response count */
	u8 beacon_probe_num[MT76_CONNAC_SCAN_DONE_EVENT_MAX_CHANNEL_NUM];
	u8 mdrdy_count[MT76_CONNAC_SCAN_DONE_EVENT_MAX_CHANNEL_NUM];
	__le32 beacon_2g_num;
	__le32 beacon_5g_num;
} __packed;

struct mt76_connac_sched_scan_req {
	u8 version;
	u8 seq_num;
	u8 stop_on_match;
	u8 ssids_num;
	u8 match_num;
	u8 pad;
	__le16 ie_len;
	struct mt76_connac_mcu_scan_ssid ssids[MT76_CONNAC_MAX_SCHED_SCAN_SSID];
	struct mt76_connac_mcu_scan_match match[MT76_CONNAC_MAX_SCAN_MATCH];
	u8 channel_type;
	u8 channels_num;
	u8 intervals_num;
	u8 scan_func; /* MT7663: BIT(0) eable random mac address */
	struct mt76_connac_mcu_scan_channel channels[64];
	__le16 intervals[MT76_CONNAC_MAX_NUM_SCHED_SCAN_INTERVAL];
	union {
		struct {
			u8 random_mac[ETH_ALEN];
			u8 pad2[58];
		} mt7663;
		struct {
			u8 bss_idx;
			u8 pad1[3];
			__le32 delay;
			u8 pad2[12];
			u8 random_mac[ETH_ALEN];
			u8 pad3[38];
		} mt7921;
	};
} __packed;

struct mt76_connac_sched_scan_done {
	u8 seq_num;
	u8 status; /* 0: ssid found */
	__le16 pad;
} __packed;

struct bss_info_uni_bss_color {
	__le16 tag;
	__le16 len;
	u8 enable;
	u8 bss_color;
	u8 rsv[2];
} __packed;

struct bss_info_uni_he {
	__le16 tag;
	__le16 len;
	__le16 he_rts_thres;
	u8 he_pe_duration;
	u8 su_disable;
	__le16 max_nss_mcs[CMD_HE_MCS_BW_NUM];
	u8 rsv[2];
} __packed;

struct mt76_connac_gtk_rekey_tlv {
	__le16 tag;
	__le16 len;
	u8 kek[NL80211_KEK_LEN];
	u8 kck[NL80211_KCK_LEN];
	u8 replay_ctr[NL80211_REPLAY_CTR_LEN];
	u8 rekey_mode; /* 0: rekey offload enable
			* 1: rekey offload disable
			* 2: rekey update
			*/
	u8 keyid;
	u8 option; /* 1: rekey data update without enabling offload */
	u8 pad[1];
	__le32 proto; /* WPA-RSN-WAPI-OPSN */
	__le32 pairwise_cipher;
	__le32 group_cipher;
	__le32 key_mgmt; /* NONE-PSK-IEEE802.1X */
	__le32 mgmt_group_cipher;
	u8 reserverd[4];
} __packed;

#define MT76_CONNAC_WOW_MASK_MAX_LEN			16
#define MT76_CONNAC_WOW_PATTEN_MAX_LEN			128

struct mt76_connac_wow_pattern_tlv {
	__le16 tag;
	__le16 len;
	u8 index; /* pattern index */
	u8 enable; /* 0: disable
		    * 1: enable
		    */
	u8 data_len; /* pattern length */
	u8 pad;
	u8 mask[MT76_CONNAC_WOW_MASK_MAX_LEN];
	u8 pattern[MT76_CONNAC_WOW_PATTEN_MAX_LEN];
	u8 rsv[4];
} __packed;

struct mt76_connac_wow_ctrl_tlv {
	__le16 tag;
	__le16 len;
	u8 cmd; /* 0x1: PM_WOWLAN_REQ_START
		 * 0x2: PM_WOWLAN_REQ_STOP
		 * 0x3: PM_WOWLAN_PARAM_CLEAR
		 */
	u8 trigger; /* 0: NONE
		     * BIT(0): NL80211_WOWLAN_TRIG_MAGIC_PKT
		     * BIT(1): NL80211_WOWLAN_TRIG_ANY
		     * BIT(2): NL80211_WOWLAN_TRIG_DISCONNECT
		     * BIT(3): NL80211_WOWLAN_TRIG_GTK_REKEY_FAILURE
		     * BIT(4): BEACON_LOST
		     * BIT(5): NL80211_WOWLAN_TRIG_NET_DETECT
		     */
	u8 wakeup_hif; /* 0x0: HIF_SDIO
			* 0x1: HIF_USB
			* 0x2: HIF_PCIE
			* 0x3: HIF_GPIO
			*/
	u8 pad;
	u8 rsv[4];
} __packed;

struct mt76_connac_wow_gpio_param_tlv {
	__le16 tag;
	__le16 len;
	u8 gpio_pin;
	u8 trigger_lvl;
	u8 pad[2];
	__le32 gpio_interval;
	u8 rsv[4];
} __packed;

struct mt76_connac_arpns_tlv {
	__le16 tag;
	__le16 len;
	u8 mode;
	u8 ips_num;
	u8 option;
	u8 pad[1];
} __packed;

struct mt76_connac_suspend_tlv {
	__le16 tag;
	__le16 len;
	u8 enable; /* 0: suspend mode disabled
		    * 1: suspend mode enabled
		    */
	u8 mdtim; /* LP parameter */
	u8 wow_suspend; /* 0: update by origin policy
			 * 1: update by wow dtim
			 */
	u8 pad[5];
} __packed;

enum mt76_sta_info_state {
	MT76_STA_INFO_STATE_NONE,
	MT76_STA_INFO_STATE_AUTH,
	MT76_STA_INFO_STATE_ASSOC
};

struct mt76_sta_cmd_info {
	struct ieee80211_sta *sta;
	struct mt76_wcid *wcid;

	struct ieee80211_vif *vif;

	bool offload_fw;
	bool enable;
	bool newly;
	int cmd;
	u8 rcpi;
	u8 state;
};

#define MT_SKU_POWER_LIMIT	161

struct mt76_connac_sku_tlv {
	u8 channel;
	s8 pwr_limit[MT_SKU_POWER_LIMIT];
} __packed;

struct mt76_connac_tx_power_limit_tlv {
	/* DW0 - common info*/
	u8 ver;
	u8 pad0;
	__le16 len;
	/* DW1 - cmd hint */
	u8 n_chan; /* # channel */
	u8 band; /* 2.4GHz - 5GHz - 6GHz */
	u8 last_msg;
	u8 pad1;
	/* DW3 */
	u8 alpha2[4]; /* regulatory_request.alpha2 */
	u8 pad2[32];
} __packed;

struct mt76_connac_config {
	__le16 id;
	u8 type;
	u8 resp_type;
	__le16 data_size;
	__le16 resv;
	u8 data[320];
} __packed;

#define to_wcid_lo(id)		FIELD_GET(GENMASK(7, 0), (u16)id)
#define to_wcid_hi(id)		FIELD_GET(GENMASK(9, 8), (u16)id)

static inline void
mt76_connac_mcu_get_wlan_idx(struct mt76_dev *dev, struct mt76_wcid *wcid,
			     u8 *wlan_idx_lo, u8 *wlan_idx_hi)
{
	*wlan_idx_hi = 0;

	if (is_mt7921(dev)) {
		*wlan_idx_lo = wcid ? to_wcid_lo(wcid->idx) : 0;
		*wlan_idx_hi = wcid ? to_wcid_hi(wcid->idx) : 0;
	} else {
		*wlan_idx_lo = wcid ? wcid->idx : 0;
	}
}

struct sk_buff *
mt76_connac_mcu_alloc_sta_req(struct mt76_dev *dev, struct mt76_vif *mvif,
			      struct mt76_wcid *wcid);
struct wtbl_req_hdr *
mt76_connac_mcu_alloc_wtbl_req(struct mt76_dev *dev, struct mt76_wcid *wcid,
			       int cmd, void *sta_wtbl, struct sk_buff **skb);
struct tlv *mt76_connac_mcu_add_nested_tlv(struct sk_buff *skb, int tag,
					   int len, void *sta_ntlv,
					   void *sta_wtbl);
static inline struct tlv *
mt76_connac_mcu_add_tlv(struct sk_buff *skb, int tag, int len)
{
	return mt76_connac_mcu_add_nested_tlv(skb, tag, len, skb->data, NULL);
}

int mt76_connac_mcu_set_channel_domain(struct mt76_phy *phy);
int mt76_connac_mcu_set_vif_ps(struct mt76_dev *dev, struct ieee80211_vif *vif);
void mt76_connac_mcu_sta_basic_tlv(struct sk_buff *skb,
				   struct ieee80211_vif *vif,
				   struct ieee80211_sta *sta, bool enable,
				   bool newly);
void mt76_connac_mcu_wtbl_generic_tlv(struct mt76_dev *dev, struct sk_buff *skb,
				      struct ieee80211_vif *vif,
				      struct ieee80211_sta *sta, void *sta_wtbl,
				      void *wtbl_tlv);
void mt76_connac_mcu_wtbl_hdr_trans_tlv(struct sk_buff *skb,
					struct ieee80211_vif *vif,
					struct mt76_wcid *wcid,
					void *sta_wtbl, void *wtbl_tlv);
int mt76_connac_mcu_sta_update_hdr_trans(struct mt76_dev *dev,
					 struct ieee80211_vif *vif,
					 struct mt76_wcid *wcid, int cmd);
void mt76_connac_mcu_sta_tlv(struct mt76_phy *mphy, struct sk_buff *skb,
			     struct ieee80211_sta *sta,
			     struct ieee80211_vif *vif,
			     u8 rcpi, u8 state);
void mt76_connac_mcu_wtbl_ht_tlv(struct mt76_dev *dev, struct sk_buff *skb,
				 struct ieee80211_sta *sta, void *sta_wtbl,
				 void *wtbl_tlv);
void mt76_connac_mcu_wtbl_ba_tlv(struct mt76_dev *dev, struct sk_buff *skb,
				 struct ieee80211_ampdu_params *params,
				 bool enable, bool tx, void *sta_wtbl,
				 void *wtbl_tlv);
void mt76_connac_mcu_sta_ba_tlv(struct sk_buff *skb,
				struct ieee80211_ampdu_params *params,
				bool enable, bool tx);
int mt76_connac_mcu_uni_add_dev(struct mt76_phy *phy,
				struct ieee80211_vif *vif,
				struct mt76_wcid *wcid,
				bool enable);
int mt76_connac_mcu_sta_ba(struct mt76_dev *dev, struct mt76_vif *mvif,
			   struct ieee80211_ampdu_params *params,
			   bool enable, bool tx);
int mt76_connac_mcu_uni_add_bss(struct mt76_phy *phy,
				struct ieee80211_vif *vif,
				struct mt76_wcid *wcid,
				bool enable);
int mt76_connac_mcu_sta_cmd(struct mt76_phy *phy,
			    struct mt76_sta_cmd_info *info);
void mt76_connac_mcu_beacon_loss_iter(void *priv, u8 *mac,
				      struct ieee80211_vif *vif);
int mt76_connac_mcu_set_rts_thresh(struct mt76_dev *dev, u32 val, u8 band);
int mt76_connac_mcu_set_mac_enable(struct mt76_dev *dev, int band, bool enable,
				   bool hdr_trans);
int mt76_connac_mcu_init_download(struct mt76_dev *dev, u32 addr, u32 len,
				  u32 mode);
int mt76_connac_mcu_start_patch(struct mt76_dev *dev);
int mt76_connac_mcu_patch_sem_ctrl(struct mt76_dev *dev, bool get);
int mt76_connac_mcu_start_firmware(struct mt76_dev *dev, u32 addr, u32 option);
int mt76_connac_mcu_get_nic_capability(struct mt76_phy *phy);

int mt76_connac_mcu_hw_scan(struct mt76_phy *phy, struct ieee80211_vif *vif,
			    struct ieee80211_scan_request *scan_req);
int mt76_connac_mcu_cancel_hw_scan(struct mt76_phy *phy,
				   struct ieee80211_vif *vif);
int mt76_connac_mcu_sched_scan_req(struct mt76_phy *phy,
				   struct ieee80211_vif *vif,
				   struct cfg80211_sched_scan_request *sreq);
int mt76_connac_mcu_sched_scan_enable(struct mt76_phy *phy,
				      struct ieee80211_vif *vif,
				      bool enable);
int mt76_connac_mcu_update_arp_filter(struct mt76_dev *dev,
				      struct mt76_vif *vif,
				      struct ieee80211_bss_conf *info);
int mt76_connac_mcu_update_gtk_rekey(struct ieee80211_hw *hw,
				     struct ieee80211_vif *vif,
				     struct cfg80211_gtk_rekey_data *key);
int mt76_connac_mcu_set_hif_suspend(struct mt76_dev *dev, bool suspend);
void mt76_connac_mcu_set_suspend_iter(void *priv, u8 *mac,
				      struct ieee80211_vif *vif);
int mt76_connac_sta_state_dp(struct mt76_dev *dev,
			     enum ieee80211_sta_state old_state,
			     enum ieee80211_sta_state new_state);
int mt76_connac_mcu_chip_config(struct mt76_dev *dev);
int mt76_connac_mcu_set_deep_sleep(struct mt76_dev *dev, bool enable);
void mt76_connac_mcu_coredump_event(struct mt76_dev *dev, struct sk_buff *skb,
				    struct mt76_connac_coredump *coredump);
int mt76_connac_mcu_set_rate_txpower(struct mt76_phy *phy);
int mt76_connac_mcu_set_p2p_oppps(struct ieee80211_hw *hw,
				  struct ieee80211_vif *vif);
u32 mt76_connac_mcu_reg_rr(struct mt76_dev *dev, u32 offset);
void mt76_connac_mcu_reg_wr(struct mt76_dev *dev, u32 offset, u32 val);
#endif /* __MT76_CONNAC_MCU_H */