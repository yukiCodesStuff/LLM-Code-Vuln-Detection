struct rtl_hal_ops {
	int (*init_sw_vars) (struct ieee80211_hw *hw);
	void (*deinit_sw_vars) (struct ieee80211_hw *hw);
	void (*read_chip_version)(struct ieee80211_hw *hw);
	void (*read_eeprom_info) (struct ieee80211_hw *hw);
	void (*interrupt_recognized) (struct ieee80211_hw *hw,
				      u32 *p_inta, u32 *p_intb);
	int (*hw_init) (struct ieee80211_hw *hw);
	void (*hw_disable) (struct ieee80211_hw *hw);
	void (*hw_suspend) (struct ieee80211_hw *hw);
	void (*hw_resume) (struct ieee80211_hw *hw);
	void (*enable_interrupt) (struct ieee80211_hw *hw);
	void (*disable_interrupt) (struct ieee80211_hw *hw);
	int (*set_network_type) (struct ieee80211_hw *hw,
				 enum nl80211_iftype type);
	void (*set_chk_bssid)(struct ieee80211_hw *hw,
				bool check_bssid);
	void (*set_bw_mode) (struct ieee80211_hw *hw,
			     enum nl80211_channel_type ch_type);
	 u8(*switch_channel) (struct ieee80211_hw *hw);
	void (*set_qos) (struct ieee80211_hw *hw, int aci);
	void (*set_bcn_reg) (struct ieee80211_hw *hw);
	void (*set_bcn_intv) (struct ieee80211_hw *hw);
	void (*update_interrupt_mask) (struct ieee80211_hw *hw,
				       u32 add_msr, u32 rm_msr);
	void (*get_hw_reg) (struct ieee80211_hw *hw, u8 variable, u8 *val);
	void (*set_hw_reg) (struct ieee80211_hw *hw, u8 variable, u8 *val);
	void (*update_rate_tbl) (struct ieee80211_hw *hw,
			      struct ieee80211_sta *sta, u8 rssi_level);
	void (*update_rate_mask) (struct ieee80211_hw *hw, u8 rssi_level);
	void (*fill_tx_desc) (struct ieee80211_hw *hw,
			      struct ieee80211_hdr *hdr, u8 *pdesc_tx,
			      struct ieee80211_tx_info *info,
			      struct ieee80211_sta *sta,
			      struct sk_buff *skb, u8 hw_queue,
			      struct rtl_tcb_desc *ptcb_desc);
	void (*fill_fake_txdesc) (struct ieee80211_hw *hw, u8 *pDesc,
				  u32 buffer_len, bool bIsPsPoll);
	void (*fill_tx_cmddesc) (struct ieee80211_hw *hw, u8 *pdesc,
				 bool firstseg, bool lastseg,
				 struct sk_buff *skb);
	bool (*cmd_send_packet)(struct ieee80211_hw *hw, struct sk_buff *skb);
	bool (*query_rx_desc) (struct ieee80211_hw *hw,
			       struct rtl_stats *stats,
			       struct ieee80211_rx_status *rx_status,
			       u8 *pdesc, struct sk_buff *skb);
	void (*set_channel_access) (struct ieee80211_hw *hw);
	bool (*radio_onoff_checking) (struct ieee80211_hw *hw, u8 *valid);
	void (*dm_watchdog) (struct ieee80211_hw *hw);
	void (*scan_operation_backup) (struct ieee80211_hw *hw, u8 operation);
	bool (*set_rf_power_state) (struct ieee80211_hw *hw,
				    enum rf_pwrstate rfpwr_state);
	void (*led_control) (struct ieee80211_hw *hw,
			     enum led_ctl_mode ledaction);
	void (*set_desc) (u8 *pdesc, bool istx, u8 desc_name, u8 *val);
	u32 (*get_desc) (u8 *pdesc, bool istx, u8 desc_name);
	void (*tx_polling) (struct ieee80211_hw *hw, u8 hw_queue);
	void (*enable_hw_sec) (struct ieee80211_hw *hw);
	void (*set_key) (struct ieee80211_hw *hw, u32 key_index,
			 u8 *macaddr, bool is_group, u8 enc_algo,
			 bool is_wepkey, bool clear_all);
	void (*init_sw_leds) (struct ieee80211_hw *hw);
	void (*deinit_sw_leds) (struct ieee80211_hw *hw);
	u32 (*get_bbreg) (struct ieee80211_hw *hw, u32 regaddr, u32 bitmask);
	void (*set_bbreg) (struct ieee80211_hw *hw, u32 regaddr, u32 bitmask,
			   u32 data);
	u32 (*get_rfreg) (struct ieee80211_hw *hw, enum radio_path rfpath,
			  u32 regaddr, u32 bitmask);
	void (*set_rfreg) (struct ieee80211_hw *hw, enum radio_path rfpath,
			   u32 regaddr, u32 bitmask, u32 data);
	void (*allow_all_destaddr)(struct ieee80211_hw *hw,
		bool allow_all_da, bool write_into_reg);
	void (*linked_set_reg) (struct ieee80211_hw *hw);
	void (*chk_switch_dmdp) (struct ieee80211_hw *hw);
	void (*dualmac_easy_concurrent) (struct ieee80211_hw *hw);
	void (*dualmac_switch_to_dmdp) (struct ieee80211_hw *hw);
	bool (*phy_rf6052_config) (struct ieee80211_hw *hw);
	void (*phy_rf6052_set_cck_txpower) (struct ieee80211_hw *hw,
					    u8 *powerlevel);
	void (*phy_rf6052_set_ofdm_txpower) (struct ieee80211_hw *hw,
					     u8 *ppowerlevel, u8 channel);
	bool (*config_bb_with_headerfile) (struct ieee80211_hw *hw,
					   u8 configtype);
	bool (*config_bb_with_pgheaderfile) (struct ieee80211_hw *hw,
					     u8 configtype);
	void (*phy_lc_calibrate) (struct ieee80211_hw *hw, bool is2t);
	void (*phy_set_bw_mode_callback) (struct ieee80211_hw *hw);
	void (*dm_dynamic_txpower) (struct ieee80211_hw *hw);
	void (*c2h_command_handle) (struct ieee80211_hw *hw);
	void (*bt_wifi_media_status_notify) (struct ieee80211_hw *hw,
					     bool mstate);
	void (*bt_coex_off_before_lps) (struct ieee80211_hw *hw);
};

struct rtl_intf_ops {
	/*com */
	void (*read_efuse_byte)(struct ieee80211_hw *hw, u16 _offset, u8 *pbuf);
	int (*adapter_start) (struct ieee80211_hw *hw);
	void (*adapter_stop) (struct ieee80211_hw *hw);
	bool (*check_buddy_priv)(struct ieee80211_hw *hw,
				 struct rtl_priv **buddy_priv);

	int (*adapter_tx) (struct ieee80211_hw *hw,
			   struct ieee80211_sta *sta,
			   struct sk_buff *skb,
			   struct rtl_tcb_desc *ptcb_desc);
	void (*flush)(struct ieee80211_hw *hw, bool drop);
	int (*reset_trx_ring) (struct ieee80211_hw *hw);
	bool (*waitq_insert) (struct ieee80211_hw *hw,
			      struct ieee80211_sta *sta,
			      struct sk_buff *skb);

	/*pci */
	void (*disable_aspm) (struct ieee80211_hw *hw);
	void (*enable_aspm) (struct ieee80211_hw *hw);

	/*usb */
};

struct rtl_mod_params {
	/* default: 0 = using hardware encryption */
	bool sw_crypto;

	/* default: 0 = DBG_EMERG (0)*/
	int debug;

	/* default: 1 = using no linked power save */
	bool inactiveps;

	/* default: 1 = using linked sw power save */
	bool swctrl_lps;

	/* default: 1 = using linked fw power save */
	bool fwctrl_lps;
};

struct rtl_hal_usbint_cfg {
	/* data - rx */
	u32 in_ep_num;
	u32 rx_urb_num;
	u32 rx_max_size;

	/* op - rx */
	void (*usb_rx_hdl)(struct ieee80211_hw *, struct sk_buff *);
	void (*usb_rx_segregate_hdl)(struct ieee80211_hw *, struct sk_buff *,
				     struct sk_buff_head *);

	/* tx */
	void (*usb_tx_cleanup)(struct ieee80211_hw *, struct sk_buff *);
	int (*usb_tx_post_hdl)(struct ieee80211_hw *, struct urb *,
			       struct sk_buff *);
	struct sk_buff *(*usb_tx_aggregate_hdl)(struct ieee80211_hw *,
						struct sk_buff_head *);

	/* endpoint mapping */
	int (*usb_endpoint_mapping)(struct ieee80211_hw *hw);
	u16 (*usb_mq_to_hwq)(__le16 fc, u16 mac80211_queue_index);
};

struct rtl_hal_cfg {
	u8 bar_id;
	bool write_readback;
	char *name;
	char *fw_name;
	struct rtl_hal_ops *ops;
	struct rtl_mod_params *mod_params;
	struct rtl_hal_usbint_cfg *usb_interface_cfg;

	/*this map used for some registers or vars
	   defined int HAL but used in MAIN */
	u32 maps[RTL_VAR_MAP_MAX];

};

struct rtl_locks {
	/* mutex */
	struct mutex conf_mutex;
	struct mutex ps_mutex;

	/*spin lock */
	spinlock_t ips_lock;
	spinlock_t irq_th_lock;
	spinlock_t irq_pci_lock;
	spinlock_t tx_lock;
	spinlock_t h2c_lock;
	spinlock_t rf_ps_lock;
	spinlock_t rf_lock;
	spinlock_t lps_lock;
	spinlock_t waitq_lock;
	spinlock_t entry_list_lock;
	spinlock_t usb_lock;

	/*FW clock change */
	spinlock_t fw_ps_lock;

	/*Dual mac*/
	spinlock_t cck_and_rw_pagea_lock;

	/*Easy concurrent*/
	spinlock_t check_sendpkt_lock;
};

struct rtl_works {
	struct ieee80211_hw *hw;

	/*timer */
	struct timer_list watchdog_timer;
	struct timer_list dualmac_easyconcurrent_retrytimer;
	struct timer_list fw_clockoff_timer;
	struct timer_list fast_antenna_training_timer;
	/*task */
	struct tasklet_struct irq_tasklet;
	struct tasklet_struct irq_prepare_bcn_tasklet;

	/*work queue */
	struct workqueue_struct *rtl_wq;
	struct delayed_work watchdog_wq;
	struct delayed_work ips_nic_off_wq;

	/* For SW LPS */
	struct delayed_work ps_work;
	struct delayed_work ps_rfon_wq;
	struct delayed_work fwevt_wq;

	struct work_struct lps_change_work;
};

struct rtl_debug {
	u32 dbgp_type[DBGP_TYPE_MAX];
	int global_debuglevel;
	u64 global_debugcomponents;

	/* add for proc debug */
	struct proc_dir_entry *proc_dir;
	char proc_name[20];
};

#define MIMO_PS_STATIC			0
#define MIMO_PS_DYNAMIC			1
#define MIMO_PS_NOLIMIT			3

struct rtl_dualmac_easy_concurrent_ctl {
	enum band_type currentbandtype_backfordmdp;
	bool close_bbandrf_for_dmsp;
	bool change_to_dmdp;
	bool change_to_dmsp;
	bool switch_in_process;
};

struct rtl_dmsp_ctl {
	bool activescan_for_slaveofdmsp;
	bool scan_for_anothermac_fordmsp;
	bool scan_for_itself_fordmsp;
	bool writedig_for_anothermacofdmsp;
	u32 curdigvalue_for_anothermacofdmsp;
	bool changecckpdstate_for_anothermacofdmsp;
	u8 curcckpdstate_for_anothermacofdmsp;
	bool changetxhighpowerlvl_for_anothermacofdmsp;
	u8 curtxhighlvl_for_anothermacofdmsp;
	long rssivalmin_for_anothermacofdmsp;
};

struct ps_t {
	u8 pre_ccastate;
	u8 cur_ccasate;
	u8 pre_rfstate;
	u8 cur_rfstate;
	long rssi_val_min;
};

struct dig_t {
	u32 rssi_lowthresh;
	u32 rssi_highthresh;
	u32 fa_lowthresh;
	u32 fa_highthresh;
	long last_min_undec_pwdb_for_dm;
	long rssi_highpower_lowthresh;
	long rssi_highpower_highthresh;
	u32 recover_cnt;
	u32 pre_igvalue;
	u32 cur_igvalue;
	long rssi_val;
	u8 dig_enable_flag;
	u8 dig_ext_port_stage;
	u8 dig_algorithm;
	u8 dig_twoport_algorithm;
	u8 dig_dbgmode;
	u8 dig_slgorithm_switch;
	u8 cursta_cstate;
	u8 presta_cstate;
	u8 curmultista_cstate;
	char back_val;
	char back_range_max;
	char back_range_min;
	u8 rx_gain_max;
	u8 rx_gain_min;
	u8 min_undec_pwdb_for_dm;
	u8 rssi_val_min;
	u8 pre_cck_cca_thres;
	u8 cur_cck_cca_thres;
	u8 pre_cck_pd_state;
	u8 cur_cck_pd_state;
	u8 pre_cck_fa_state;
	u8 cur_cck_fa_state;
	u8 pre_ccastate;
	u8 cur_ccasate;
	u8 large_fa_hit;
	u8 forbidden_igi;
	u8 dig_state;
	u8 dig_highpwrstate;
	u8 cur_sta_cstate;
	u8 pre_sta_cstate;
	u8 cur_ap_cstate;
	u8 pre_ap_cstate;
	u8 cur_pd_thstate;
	u8 pre_pd_thstate;
	u8 cur_cs_ratiostate;
	u8 pre_cs_ratiostate;
	u8 backoff_enable_flag;
	char backoffval_range_max;
	char backoffval_range_min;
	u8 dig_min_0;
	u8 dig_min_1;
	bool media_connect_0;
	bool media_connect_1;

	u32 antdiv_rssi_max;
	u32 rssi_max;
};

struct rtl_global_var {
	/* from this list we can get
	 * other adapter's rtl_priv */
	struct list_head glb_priv_list;
	spinlock_t glb_list_lock;
};

struct rtl_priv {
	struct ieee80211_hw *hw;
	struct completion firmware_loading_complete;
	struct list_head list;
	struct rtl_priv *buddy_priv;
	struct rtl_global_var *glb_var;
	struct rtl_dualmac_easy_concurrent_ctl easy_concurrent_ctl;
	struct rtl_dmsp_ctl dmsp_ctl;
	struct rtl_locks locks;
	struct rtl_works works;
	struct rtl_mac mac80211;
	struct rtl_hal rtlhal;
	struct rtl_regulatory regd;
	struct rtl_rfkill rfkill;
	struct rtl_io io;
	struct rtl_phy phy;
	struct rtl_dm dm;
	struct rtl_security sec;
	struct rtl_efuse efuse;

	struct rtl_ps_ctl psc;
	struct rate_adaptive ra;
	struct wireless_stats stats;
	struct rt_link_detect link_info;
	struct false_alarm_statistics falsealm_cnt;

	struct rtl_rate_priv *rate_priv;

	/* sta entry list for ap adhoc or mesh */
	struct list_head entry_list;

	struct rtl_debug dbg;
	int max_fw_size;

	/*
	 *hal_cfg : for diff cards
	 *intf_ops : for diff interrface usb/pcie
	 */
	struct rtl_hal_cfg *cfg;
	struct rtl_intf_ops *intf_ops;

	/*this var will be set by set_bit,
	   and was used to indicate status of
	   interface or hardware */
	unsigned long status;

	/* tables for dm */
	struct dig_t dm_digtable;
	struct ps_t dm_pstable;

	/* section shared by individual drivers */
	union {
		struct {	/* data buffer pointer for USB reads */
			__le32 *usb_data;
			int usb_data_index;
			bool initialized;
		};
		struct {	/* section for 8723ae */
			bool reg_init;	/* true if regs saved */
			u32 reg_874;
			u32 reg_c70;
			u32 reg_85c;
			u32 reg_a74;
			bool bt_operation_on;
		};
	};
	bool enter_ps;	/* true when entering PS */

	/*This must be the last item so
	   that it points to the data allocated
	   beyond  this structure like:
	   rtl_pci_priv or rtl_usb_priv */
	u8 priv[0];
};

#define rtl_priv(hw)		(((struct rtl_priv *)(hw)->priv))
#define rtl_mac(rtlpriv)	(&((rtlpriv)->mac80211))
#define rtl_hal(rtlpriv)	(&((rtlpriv)->rtlhal))
#define rtl_efuse(rtlpriv)	(&((rtlpriv)->efuse))
#define rtl_psc(rtlpriv)	(&((rtlpriv)->psc))


/***************************************
    Bluetooth Co-existence Related
****************************************/

enum bt_ant_num {
	ANT_X2 = 0,
	ANT_X1 = 1,
};

enum bt_co_type {
	BT_2WIRE = 0,
	BT_ISSC_3WIRE = 1,
	BT_ACCEL = 2,
	BT_CSR_BC4 = 3,
	BT_CSR_BC8 = 4,
	BT_RTL8756 = 5,
	BT_RTL8723A = 6,
};

enum bt_cur_state {
	BT_OFF = 0,
	BT_ON = 1,
};

enum bt_service_type {
	BT_SCO = 0,
	BT_A2DP = 1,
	BT_HID = 2,
	BT_HID_IDLE = 3,
	BT_SCAN = 4,
	BT_IDLE = 5,
	BT_OTHER_ACTION = 6,
	BT_BUSY = 7,
	BT_OTHERBUSY = 8,
	BT_PAN = 9,
};

enum bt_radio_shared {
	BT_RADIO_SHARED = 0,
	BT_RADIO_INDIVIDUAL = 1,
};

struct bt_coexist_info {

	/* EEPROM BT info. */
	u8 eeprom_bt_coexist;
	u8 eeprom_bt_type;
	u8 eeprom_bt_ant_num;
	u8 eeprom_bt_ant_isol;
	u8 eeprom_bt_radio_shared;

	u8 bt_coexistence;
	u8 bt_ant_num;
	u8 bt_coexist_type;
	u8 bt_state;
	u8 bt_cur_state;	/* 0:on, 1:off */
	u8 bt_ant_isolation;	/* 0:good, 1:bad */
	u8 bt_pape_ctrl;	/* 0:SW, 1:SW/HW dynamic */
	u8 bt_service;
	u8 bt_radio_shared_type;
	u8 bt_rfreg_origin_1e;
	u8 bt_rfreg_origin_1f;
	u8 bt_rssi_state;
	u32 ratio_tx;
	u32 ratio_pri;
	u32 bt_edca_ul;
	u32 bt_edca_dl;

	bool init_set;
	bool bt_busy_traffic;
	bool bt_traffic_mode_set;
	bool bt_non_traffic_mode_set;

	bool fw_coexist_all_off;
	bool sw_coexist_all_off;
	bool hw_coexist_all_off;
	u32 cstate;
	u32 previous_state;
	u32 cstate_h;
	u32 previous_state_h;

	u8 bt_pre_rssi_state;
	u8 bt_pre_rssi_state1;

	u8 reg_bt_iso;
	u8 reg_bt_sco;
	bool balance_on;
	u8 bt_active_zero_cnt;
	bool cur_bt_disabled;
	bool pre_bt_disabled;

	u8 bt_profile_case;
	u8 bt_profile_action;
	bool bt_busy;
	bool hold_for_bt_operation;
	u8 lps_counter;
};


/****************************************
	mem access macro define start
	Call endian free function when
	1. Read/write packet content.
	2. Before write integer to IO.
	3. After read integer from IO.
****************************************/
/* Convert little data endian to host ordering */
#define EF1BYTE(_val)		\
	((u8)(_val))
#define EF2BYTE(_val)		\
	(le16_to_cpu(_val))
#define EF4BYTE(_val)		\
	(le32_to_cpu(_val))

/* Read data from memory */
#define READEF1BYTE(_ptr)	\
	EF1BYTE(*((u8 *)(_ptr)))
/* Read le16 data from memory and convert to host ordering */
#define READEF2BYTE(_ptr)	\
	EF2BYTE(*(_ptr))
#define READEF4BYTE(_ptr)	\
	EF4BYTE(*(_ptr))

/* Write data to memory */
#define WRITEEF1BYTE(_ptr, _val)	\
	(*((u8 *)(_ptr))) = EF1BYTE(_val)
/* Write le16 data to memory in host ordering */
#define WRITEEF2BYTE(_ptr, _val)	\
	(*((u16 *)(_ptr))) = EF2BYTE(_val)
#define WRITEEF4BYTE(_ptr, _val)	\
	(*((u32 *)(_ptr))) = EF2BYTE(_val)

/* Create a bit mask
 * Examples:
 * BIT_LEN_MASK_32(0) => 0x00000000
 * BIT_LEN_MASK_32(1) => 0x00000001
 * BIT_LEN_MASK_32(2) => 0x00000003
 * BIT_LEN_MASK_32(32) => 0xFFFFFFFF
 */
#define BIT_LEN_MASK_32(__bitlen)	 \
	(0xFFFFFFFF >> (32 - (__bitlen)))
#define BIT_LEN_MASK_16(__bitlen)	 \
	(0xFFFF >> (16 - (__bitlen)))
#define BIT_LEN_MASK_8(__bitlen) \
	(0xFF >> (8 - (__bitlen)))

/* Create an offset bit mask
 * Examples:
 * BIT_OFFSET_LEN_MASK_32(0, 2) => 0x00000003
 * BIT_OFFSET_LEN_MASK_32(16, 2) => 0x00030000
 */
#define BIT_OFFSET_LEN_MASK_32(__bitoffset, __bitlen) \
	(BIT_LEN_MASK_32(__bitlen) << (__bitoffset))
#define BIT_OFFSET_LEN_MASK_16(__bitoffset, __bitlen) \
	(BIT_LEN_MASK_16(__bitlen) << (__bitoffset))
#define BIT_OFFSET_LEN_MASK_8(__bitoffset, __bitlen) \
	(BIT_LEN_MASK_8(__bitlen) << (__bitoffset))

/*Description:
 * Return 4-byte value in host byte ordering from
 * 4-byte pointer in little-endian system.
 */
#define LE_P4BYTE_TO_HOST_4BYTE(__pstart) \
	(EF4BYTE(*((__le32 *)(__pstart))))
#define LE_P2BYTE_TO_HOST_2BYTE(__pstart) \
	(EF2BYTE(*((__le16 *)(__pstart))))
#define LE_P1BYTE_TO_HOST_1BYTE(__pstart) \
	(EF1BYTE(*((u8 *)(__pstart))))

/*Description:
Translate subfield (continuous bits in little-endian) of 4-byte
value to host byte ordering.*/
#define LE_BITS_TO_4BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		(LE_P4BYTE_TO_HOST_4BYTE(__pstart) >> (__bitoffset))  & \
		BIT_LEN_MASK_32(__bitlen) \
	)
#define LE_BITS_TO_2BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		(LE_P2BYTE_TO_HOST_2BYTE(__pstart) >> (__bitoffset)) & \
		BIT_LEN_MASK_16(__bitlen) \
	)
#define LE_BITS_TO_1BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		(LE_P1BYTE_TO_HOST_1BYTE(__pstart) >> (__bitoffset)) & \
		BIT_LEN_MASK_8(__bitlen) \
	)

/* Description:
 * Mask subfield (continuous bits in little-endian) of 4-byte value
 * and return the result in 4-byte value in host byte ordering.
 */
#define LE_BITS_CLEARED_TO_4BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		LE_P4BYTE_TO_HOST_4BYTE(__pstart)  & \
		(~BIT_OFFSET_LEN_MASK_32(__bitoffset, __bitlen)) \
	)
#define LE_BITS_CLEARED_TO_2BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		LE_P2BYTE_TO_HOST_2BYTE(__pstart) & \
		(~BIT_OFFSET_LEN_MASK_16(__bitoffset, __bitlen)) \
	)
#define LE_BITS_CLEARED_TO_1BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		LE_P1BYTE_TO_HOST_1BYTE(__pstart) & \
		(~BIT_OFFSET_LEN_MASK_8(__bitoffset, __bitlen)) \
	)

/* Description:
 * Set subfield of little-endian 4-byte value to specified value.
 */
#define SET_BITS_TO_LE_4BYTE(__pstart, __bitoffset, __bitlen, __val) \
	*((u32 *)(__pstart)) = \
	( \
		LE_BITS_CLEARED_TO_4BYTE(__pstart, __bitoffset, __bitlen) | \
		((((u32)__val) & BIT_LEN_MASK_32(__bitlen)) << (__bitoffset)) \
	);
#define SET_BITS_TO_LE_2BYTE(__pstart, __bitoffset, __bitlen, __val) \
	*((u16 *)(__pstart)) = \
	( \
		LE_BITS_CLEARED_TO_2BYTE(__pstart, __bitoffset, __bitlen) | \
		((((u16)__val) & BIT_LEN_MASK_16(__bitlen)) << (__bitoffset)) \
	);
#define SET_BITS_TO_LE_1BYTE(__pstart, __bitoffset, __bitlen, __val) \
	*((u8 *)(__pstart)) = EF1BYTE \
	( \
		LE_BITS_CLEARED_TO_1BYTE(__pstart, __bitoffset, __bitlen) | \
		((((u8)__val) & BIT_LEN_MASK_8(__bitlen)) << (__bitoffset)) \
	);

#define	N_BYTE_ALIGMENT(__value, __aligment) ((__aligment == 1) ? \
	(__value) : (((__value + __aligment - 1) / __aligment) * __aligment))

/****************************************
	mem access macro define end
****************************************/

#define byte(x, n) ((x >> (8 * n)) & 0xff)

#define packet_get_type(_packet) (EF1BYTE((_packet).octet[0]) & 0xFC)
#define RTL_WATCH_DOG_TIME	2000
#define MSECS(t)		msecs_to_jiffies(t)
#define WLAN_FC_GET_VERS(fc)	(le16_to_cpu(fc) & IEEE80211_FCTL_VERS)
#define WLAN_FC_GET_TYPE(fc)	(le16_to_cpu(fc) & IEEE80211_FCTL_FTYPE)
#define WLAN_FC_GET_STYPE(fc)	(le16_to_cpu(fc) & IEEE80211_FCTL_STYPE)
#define WLAN_FC_MORE_DATA(fc)	(le16_to_cpu(fc) & IEEE80211_FCTL_MOREDATA)
#define rtl_dm(rtlpriv)		(&((rtlpriv)->dm))

#define	RT_RF_OFF_LEVL_ASPM		BIT(0)	/*PCI ASPM */
#define	RT_RF_OFF_LEVL_CLK_REQ		BIT(1)	/*PCI clock request */
#define	RT_RF_OFF_LEVL_PCI_D3		BIT(2)	/*PCI D3 mode */
/*NIC halt, re-initialize hw parameters*/
#define	RT_RF_OFF_LEVL_HALT_NIC		BIT(3)
#define	RT_RF_OFF_LEVL_FREE_FW		BIT(4)	/*FW free, re-download the FW */
#define	RT_RF_OFF_LEVL_FW_32K		BIT(5)	/*FW in 32k */
/*Always enable ASPM and Clock Req in initialization.*/
#define	RT_RF_PS_LEVEL_ALWAYS_ASPM	BIT(6)
/* no matter RFOFF or SLEEP we set PS_ASPM_LEVL*/
#define	RT_PS_LEVEL_ASPM		BIT(7)
/*When LPS is on, disable 2R if no packet is received or transmittd.*/
#define	RT_RF_LPS_DISALBE_2R		BIT(30)
#define	RT_RF_LPS_LEVEL_ASPM		BIT(31)	/*LPS with ASPM */
#define	RT_IN_PS_LEVEL(ppsc, _ps_flg)		\
	((ppsc->cur_ps_level & _ps_flg) ? true : false)
#define	RT_CLEAR_PS_LEVEL(ppsc, _ps_flg)	\
	(ppsc->cur_ps_level &= (~(_ps_flg)))
#define	RT_SET_PS_LEVEL(ppsc, _ps_flg)		\
	(ppsc->cur_ps_level |= _ps_flg)

#define container_of_dwork_rtl(x, y, z) \
	container_of(container_of(x, struct delayed_work, work), y, z)

#define FILL_OCTET_STRING(_os, _octet, _len)	\
		(_os).octet = (u8 *)(_octet);		\
		(_os).length = (_len);

#define CP_MACADDR(des, src)	\
	((des)[0] = (src)[0], (des)[1] = (src)[1],\
	(des)[2] = (src)[2], (des)[3] = (src)[3],\
	(des)[4] = (src)[4], (des)[5] = (src)[5])

static inline u8 rtl_read_byte(struct rtl_priv *rtlpriv, u32 addr)
{
	return rtlpriv->io.read8_sync(rtlpriv, addr);
}

static inline u16 rtl_read_word(struct rtl_priv *rtlpriv, u32 addr)
{
	return rtlpriv->io.read16_sync(rtlpriv, addr);
}

static inline u32 rtl_read_dword(struct rtl_priv *rtlpriv, u32 addr)
{
	return rtlpriv->io.read32_sync(rtlpriv, addr);
}

static inline void rtl_write_byte(struct rtl_priv *rtlpriv, u32 addr, u8 val8)
{
	rtlpriv->io.write8_async(rtlpriv, addr, val8);

	if (rtlpriv->cfg->write_readback)
		rtlpriv->io.read8_sync(rtlpriv, addr);
}

static inline void rtl_write_word(struct rtl_priv *rtlpriv, u32 addr, u16 val16)
{
	rtlpriv->io.write16_async(rtlpriv, addr, val16);

	if (rtlpriv->cfg->write_readback)
		rtlpriv->io.read16_sync(rtlpriv, addr);
}

static inline void rtl_write_dword(struct rtl_priv *rtlpriv,
				   u32 addr, u32 val32)
{
	rtlpriv->io.write32_async(rtlpriv, addr, val32);

	if (rtlpriv->cfg->write_readback)
		rtlpriv->io.read32_sync(rtlpriv, addr);
}

static inline u32 rtl_get_bbreg(struct ieee80211_hw *hw,
				u32 regaddr, u32 bitmask)
{
	struct rtl_priv *rtlpriv = hw->priv;

	return rtlpriv->cfg->ops->get_bbreg(hw, regaddr, bitmask);
}

static inline void rtl_set_bbreg(struct ieee80211_hw *hw, u32 regaddr,
				 u32 bitmask, u32 data)
{
	struct rtl_priv *rtlpriv = hw->priv;

	rtlpriv->cfg->ops->set_bbreg(hw, regaddr, bitmask, data);
}

static inline u32 rtl_get_rfreg(struct ieee80211_hw *hw,
				enum radio_path rfpath, u32 regaddr,
				u32 bitmask)
{
	struct rtl_priv *rtlpriv = hw->priv;

	return rtlpriv->cfg->ops->get_rfreg(hw, rfpath, regaddr, bitmask);
}

static inline void rtl_set_rfreg(struct ieee80211_hw *hw,
				 enum radio_path rfpath, u32 regaddr,
				 u32 bitmask, u32 data)
{
	struct rtl_priv *rtlpriv = hw->priv;

	rtlpriv->cfg->ops->set_rfreg(hw, rfpath, regaddr, bitmask, data);
}

static inline bool is_hal_stop(struct rtl_hal *rtlhal)
{
	return (_HAL_STATE_STOP == rtlhal->state);
}

static inline void set_hal_start(struct rtl_hal *rtlhal)
{
	rtlhal->state = _HAL_STATE_START;
}

static inline void set_hal_stop(struct rtl_hal *rtlhal)
{
	rtlhal->state = _HAL_STATE_STOP;
}

static inline u8 get_rf_type(struct rtl_phy *rtlphy)
{
	return rtlphy->rf_type;
}

static inline struct ieee80211_hdr *rtl_get_hdr(struct sk_buff *skb)
{
	return (struct ieee80211_hdr *)(skb->data);
}

static inline __le16 rtl_get_fc(struct sk_buff *skb)
{
	return rtl_get_hdr(skb)->frame_control;
}

static inline u16 rtl_get_tid_h(struct ieee80211_hdr *hdr)
{
	return (ieee80211_get_qos_ctl(hdr))[0] & IEEE80211_QOS_CTL_TID_MASK;
}

static inline u16 rtl_get_tid(struct sk_buff *skb)
{
	return rtl_get_tid_h(rtl_get_hdr(skb));
}

static inline struct ieee80211_sta *get_sta(struct ieee80211_hw *hw,
					    struct ieee80211_vif *vif,
					    const u8 *bssid)
{
	return ieee80211_find_sta(vif, bssid);
}

static inline struct ieee80211_sta *rtl_find_sta(struct ieee80211_hw *hw,
		u8 *mac_addr)
{
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	return ieee80211_find_sta(mac->vif, mac_addr);
}

#endif
struct rtl_works {
	struct ieee80211_hw *hw;

	/*timer */
	struct timer_list watchdog_timer;
	struct timer_list dualmac_easyconcurrent_retrytimer;
	struct timer_list fw_clockoff_timer;
	struct timer_list fast_antenna_training_timer;
	/*task */
	struct tasklet_struct irq_tasklet;
	struct tasklet_struct irq_prepare_bcn_tasklet;

	/*work queue */
	struct workqueue_struct *rtl_wq;
	struct delayed_work watchdog_wq;
	struct delayed_work ips_nic_off_wq;

	/* For SW LPS */
	struct delayed_work ps_work;
	struct delayed_work ps_rfon_wq;
	struct delayed_work fwevt_wq;

	struct work_struct lps_change_work;
};

struct rtl_debug {
	u32 dbgp_type[DBGP_TYPE_MAX];
	int global_debuglevel;
	u64 global_debugcomponents;

	/* add for proc debug */
	struct proc_dir_entry *proc_dir;
	char proc_name[20];
};

#define MIMO_PS_STATIC			0
#define MIMO_PS_DYNAMIC			1
#define MIMO_PS_NOLIMIT			3

struct rtl_dualmac_easy_concurrent_ctl {
	enum band_type currentbandtype_backfordmdp;
	bool close_bbandrf_for_dmsp;
	bool change_to_dmdp;
	bool change_to_dmsp;
	bool switch_in_process;
};

struct rtl_dmsp_ctl {
	bool activescan_for_slaveofdmsp;
	bool scan_for_anothermac_fordmsp;
	bool scan_for_itself_fordmsp;
	bool writedig_for_anothermacofdmsp;
	u32 curdigvalue_for_anothermacofdmsp;
	bool changecckpdstate_for_anothermacofdmsp;
	u8 curcckpdstate_for_anothermacofdmsp;
	bool changetxhighpowerlvl_for_anothermacofdmsp;
	u8 curtxhighlvl_for_anothermacofdmsp;
	long rssivalmin_for_anothermacofdmsp;
};

struct ps_t {
	u8 pre_ccastate;
	u8 cur_ccasate;
	u8 pre_rfstate;
	u8 cur_rfstate;
	long rssi_val_min;
};

struct dig_t {
	u32 rssi_lowthresh;
	u32 rssi_highthresh;
	u32 fa_lowthresh;
	u32 fa_highthresh;
	long last_min_undec_pwdb_for_dm;
	long rssi_highpower_lowthresh;
	long rssi_highpower_highthresh;
	u32 recover_cnt;
	u32 pre_igvalue;
	u32 cur_igvalue;
	long rssi_val;
	u8 dig_enable_flag;
	u8 dig_ext_port_stage;
	u8 dig_algorithm;
	u8 dig_twoport_algorithm;
	u8 dig_dbgmode;
	u8 dig_slgorithm_switch;
	u8 cursta_cstate;
	u8 presta_cstate;
	u8 curmultista_cstate;
	char back_val;
	char back_range_max;
	char back_range_min;
	u8 rx_gain_max;
	u8 rx_gain_min;
	u8 min_undec_pwdb_for_dm;
	u8 rssi_val_min;
	u8 pre_cck_cca_thres;
	u8 cur_cck_cca_thres;
	u8 pre_cck_pd_state;
	u8 cur_cck_pd_state;
	u8 pre_cck_fa_state;
	u8 cur_cck_fa_state;
	u8 pre_ccastate;
	u8 cur_ccasate;
	u8 large_fa_hit;
	u8 forbidden_igi;
	u8 dig_state;
	u8 dig_highpwrstate;
	u8 cur_sta_cstate;
	u8 pre_sta_cstate;
	u8 cur_ap_cstate;
	u8 pre_ap_cstate;
	u8 cur_pd_thstate;
	u8 pre_pd_thstate;
	u8 cur_cs_ratiostate;
	u8 pre_cs_ratiostate;
	u8 backoff_enable_flag;
	char backoffval_range_max;
	char backoffval_range_min;
	u8 dig_min_0;
	u8 dig_min_1;
	bool media_connect_0;
	bool media_connect_1;

	u32 antdiv_rssi_max;
	u32 rssi_max;
};

struct rtl_global_var {
	/* from this list we can get
	 * other adapter's rtl_priv */
	struct list_head glb_priv_list;
	spinlock_t glb_list_lock;
};

struct rtl_priv {
	struct ieee80211_hw *hw;
	struct completion firmware_loading_complete;
	struct list_head list;
	struct rtl_priv *buddy_priv;
	struct rtl_global_var *glb_var;
	struct rtl_dualmac_easy_concurrent_ctl easy_concurrent_ctl;
	struct rtl_dmsp_ctl dmsp_ctl;
	struct rtl_locks locks;
	struct rtl_works works;
	struct rtl_mac mac80211;
	struct rtl_hal rtlhal;
	struct rtl_regulatory regd;
	struct rtl_rfkill rfkill;
	struct rtl_io io;
	struct rtl_phy phy;
	struct rtl_dm dm;
	struct rtl_security sec;
	struct rtl_efuse efuse;

	struct rtl_ps_ctl psc;
	struct rate_adaptive ra;
	struct wireless_stats stats;
	struct rt_link_detect link_info;
	struct false_alarm_statistics falsealm_cnt;

	struct rtl_rate_priv *rate_priv;

	/* sta entry list for ap adhoc or mesh */
	struct list_head entry_list;

	struct rtl_debug dbg;
	int max_fw_size;

	/*
	 *hal_cfg : for diff cards
	 *intf_ops : for diff interrface usb/pcie
	 */
	struct rtl_hal_cfg *cfg;
	struct rtl_intf_ops *intf_ops;

	/*this var will be set by set_bit,
	   and was used to indicate status of
	   interface or hardware */
	unsigned long status;

	/* tables for dm */
	struct dig_t dm_digtable;
	struct ps_t dm_pstable;

	/* section shared by individual drivers */
	union {
		struct {	/* data buffer pointer for USB reads */
			__le32 *usb_data;
			int usb_data_index;
			bool initialized;
		};
		struct {	/* section for 8723ae */
			bool reg_init;	/* true if regs saved */
			u32 reg_874;
			u32 reg_c70;
			u32 reg_85c;
			u32 reg_a74;
			bool bt_operation_on;
		};
	};
	bool enter_ps;	/* true when entering PS */

	/*This must be the last item so
	   that it points to the data allocated
	   beyond  this structure like:
	   rtl_pci_priv or rtl_usb_priv */
	u8 priv[0];
};

#define rtl_priv(hw)		(((struct rtl_priv *)(hw)->priv))
#define rtl_mac(rtlpriv)	(&((rtlpriv)->mac80211))
#define rtl_hal(rtlpriv)	(&((rtlpriv)->rtlhal))
#define rtl_efuse(rtlpriv)	(&((rtlpriv)->efuse))
#define rtl_psc(rtlpriv)	(&((rtlpriv)->psc))


/***************************************
    Bluetooth Co-existence Related
****************************************/

enum bt_ant_num {
	ANT_X2 = 0,
	ANT_X1 = 1,
};

enum bt_co_type {
	BT_2WIRE = 0,
	BT_ISSC_3WIRE = 1,
	BT_ACCEL = 2,
	BT_CSR_BC4 = 3,
	BT_CSR_BC8 = 4,
	BT_RTL8756 = 5,
	BT_RTL8723A = 6,
};

enum bt_cur_state {
	BT_OFF = 0,
	BT_ON = 1,
};

enum bt_service_type {
	BT_SCO = 0,
	BT_A2DP = 1,
	BT_HID = 2,
	BT_HID_IDLE = 3,
	BT_SCAN = 4,
	BT_IDLE = 5,
	BT_OTHER_ACTION = 6,
	BT_BUSY = 7,
	BT_OTHERBUSY = 8,
	BT_PAN = 9,
};

enum bt_radio_shared {
	BT_RADIO_SHARED = 0,
	BT_RADIO_INDIVIDUAL = 1,
};

struct bt_coexist_info {

	/* EEPROM BT info. */
	u8 eeprom_bt_coexist;
	u8 eeprom_bt_type;
	u8 eeprom_bt_ant_num;
	u8 eeprom_bt_ant_isol;
	u8 eeprom_bt_radio_shared;

	u8 bt_coexistence;
	u8 bt_ant_num;
	u8 bt_coexist_type;
	u8 bt_state;
	u8 bt_cur_state;	/* 0:on, 1:off */
	u8 bt_ant_isolation;	/* 0:good, 1:bad */
	u8 bt_pape_ctrl;	/* 0:SW, 1:SW/HW dynamic */
	u8 bt_service;
	u8 bt_radio_shared_type;
	u8 bt_rfreg_origin_1e;
	u8 bt_rfreg_origin_1f;
	u8 bt_rssi_state;
	u32 ratio_tx;
	u32 ratio_pri;
	u32 bt_edca_ul;
	u32 bt_edca_dl;

	bool init_set;
	bool bt_busy_traffic;
	bool bt_traffic_mode_set;
	bool bt_non_traffic_mode_set;

	bool fw_coexist_all_off;
	bool sw_coexist_all_off;
	bool hw_coexist_all_off;
	u32 cstate;
	u32 previous_state;
	u32 cstate_h;
	u32 previous_state_h;

	u8 bt_pre_rssi_state;
	u8 bt_pre_rssi_state1;

	u8 reg_bt_iso;
	u8 reg_bt_sco;
	bool balance_on;
	u8 bt_active_zero_cnt;
	bool cur_bt_disabled;
	bool pre_bt_disabled;

	u8 bt_profile_case;
	u8 bt_profile_action;
	bool bt_busy;
	bool hold_for_bt_operation;
	u8 lps_counter;
};


/****************************************
	mem access macro define start
	Call endian free function when
	1. Read/write packet content.
	2. Before write integer to IO.
	3. After read integer from IO.
****************************************/
/* Convert little data endian to host ordering */
#define EF1BYTE(_val)		\
	((u8)(_val))
#define EF2BYTE(_val)		\
	(le16_to_cpu(_val))
#define EF4BYTE(_val)		\
	(le32_to_cpu(_val))

/* Read data from memory */
#define READEF1BYTE(_ptr)	\
	EF1BYTE(*((u8 *)(_ptr)))
/* Read le16 data from memory and convert to host ordering */
#define READEF2BYTE(_ptr)	\
	EF2BYTE(*(_ptr))
#define READEF4BYTE(_ptr)	\
	EF4BYTE(*(_ptr))

/* Write data to memory */
#define WRITEEF1BYTE(_ptr, _val)	\
	(*((u8 *)(_ptr))) = EF1BYTE(_val)
/* Write le16 data to memory in host ordering */
#define WRITEEF2BYTE(_ptr, _val)	\
	(*((u16 *)(_ptr))) = EF2BYTE(_val)
#define WRITEEF4BYTE(_ptr, _val)	\
	(*((u32 *)(_ptr))) = EF2BYTE(_val)

/* Create a bit mask
 * Examples:
 * BIT_LEN_MASK_32(0) => 0x00000000
 * BIT_LEN_MASK_32(1) => 0x00000001
 * BIT_LEN_MASK_32(2) => 0x00000003
 * BIT_LEN_MASK_32(32) => 0xFFFFFFFF
 */
#define BIT_LEN_MASK_32(__bitlen)	 \
	(0xFFFFFFFF >> (32 - (__bitlen)))
#define BIT_LEN_MASK_16(__bitlen)	 \
	(0xFFFF >> (16 - (__bitlen)))
#define BIT_LEN_MASK_8(__bitlen) \
	(0xFF >> (8 - (__bitlen)))

/* Create an offset bit mask
 * Examples:
 * BIT_OFFSET_LEN_MASK_32(0, 2) => 0x00000003
 * BIT_OFFSET_LEN_MASK_32(16, 2) => 0x00030000
 */
#define BIT_OFFSET_LEN_MASK_32(__bitoffset, __bitlen) \
	(BIT_LEN_MASK_32(__bitlen) << (__bitoffset))
#define BIT_OFFSET_LEN_MASK_16(__bitoffset, __bitlen) \
	(BIT_LEN_MASK_16(__bitlen) << (__bitoffset))
#define BIT_OFFSET_LEN_MASK_8(__bitoffset, __bitlen) \
	(BIT_LEN_MASK_8(__bitlen) << (__bitoffset))

/*Description:
 * Return 4-byte value in host byte ordering from
 * 4-byte pointer in little-endian system.
 */
#define LE_P4BYTE_TO_HOST_4BYTE(__pstart) \
	(EF4BYTE(*((__le32 *)(__pstart))))
#define LE_P2BYTE_TO_HOST_2BYTE(__pstart) \
	(EF2BYTE(*((__le16 *)(__pstart))))
#define LE_P1BYTE_TO_HOST_1BYTE(__pstart) \
	(EF1BYTE(*((u8 *)(__pstart))))

/*Description:
Translate subfield (continuous bits in little-endian) of 4-byte
value to host byte ordering.*/
#define LE_BITS_TO_4BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		(LE_P4BYTE_TO_HOST_4BYTE(__pstart) >> (__bitoffset))  & \
		BIT_LEN_MASK_32(__bitlen) \
	)
#define LE_BITS_TO_2BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		(LE_P2BYTE_TO_HOST_2BYTE(__pstart) >> (__bitoffset)) & \
		BIT_LEN_MASK_16(__bitlen) \
	)
#define LE_BITS_TO_1BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		(LE_P1BYTE_TO_HOST_1BYTE(__pstart) >> (__bitoffset)) & \
		BIT_LEN_MASK_8(__bitlen) \
	)

/* Description:
 * Mask subfield (continuous bits in little-endian) of 4-byte value
 * and return the result in 4-byte value in host byte ordering.
 */
#define LE_BITS_CLEARED_TO_4BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		LE_P4BYTE_TO_HOST_4BYTE(__pstart)  & \
		(~BIT_OFFSET_LEN_MASK_32(__bitoffset, __bitlen)) \
	)
#define LE_BITS_CLEARED_TO_2BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		LE_P2BYTE_TO_HOST_2BYTE(__pstart) & \
		(~BIT_OFFSET_LEN_MASK_16(__bitoffset, __bitlen)) \
	)
#define LE_BITS_CLEARED_TO_1BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		LE_P1BYTE_TO_HOST_1BYTE(__pstart) & \
		(~BIT_OFFSET_LEN_MASK_8(__bitoffset, __bitlen)) \
	)

/* Description:
 * Set subfield of little-endian 4-byte value to specified value.
 */
#define SET_BITS_TO_LE_4BYTE(__pstart, __bitoffset, __bitlen, __val) \
	*((u32 *)(__pstart)) = \
	( \
		LE_BITS_CLEARED_TO_4BYTE(__pstart, __bitoffset, __bitlen) | \
		((((u32)__val) & BIT_LEN_MASK_32(__bitlen)) << (__bitoffset)) \
	);
#define SET_BITS_TO_LE_2BYTE(__pstart, __bitoffset, __bitlen, __val) \
	*((u16 *)(__pstart)) = \
	( \
		LE_BITS_CLEARED_TO_2BYTE(__pstart, __bitoffset, __bitlen) | \
		((((u16)__val) & BIT_LEN_MASK_16(__bitlen)) << (__bitoffset)) \
	);
#define SET_BITS_TO_LE_1BYTE(__pstart, __bitoffset, __bitlen, __val) \
	*((u8 *)(__pstart)) = EF1BYTE \
	( \
		LE_BITS_CLEARED_TO_1BYTE(__pstart, __bitoffset, __bitlen) | \
		((((u8)__val) & BIT_LEN_MASK_8(__bitlen)) << (__bitoffset)) \
	);

#define	N_BYTE_ALIGMENT(__value, __aligment) ((__aligment == 1) ? \
	(__value) : (((__value + __aligment - 1) / __aligment) * __aligment))

/****************************************
	mem access macro define end
****************************************/

#define byte(x, n) ((x >> (8 * n)) & 0xff)

#define packet_get_type(_packet) (EF1BYTE((_packet).octet[0]) & 0xFC)
#define RTL_WATCH_DOG_TIME	2000
#define MSECS(t)		msecs_to_jiffies(t)
#define WLAN_FC_GET_VERS(fc)	(le16_to_cpu(fc) & IEEE80211_FCTL_VERS)
#define WLAN_FC_GET_TYPE(fc)	(le16_to_cpu(fc) & IEEE80211_FCTL_FTYPE)
#define WLAN_FC_GET_STYPE(fc)	(le16_to_cpu(fc) & IEEE80211_FCTL_STYPE)
#define WLAN_FC_MORE_DATA(fc)	(le16_to_cpu(fc) & IEEE80211_FCTL_MOREDATA)
#define rtl_dm(rtlpriv)		(&((rtlpriv)->dm))

#define	RT_RF_OFF_LEVL_ASPM		BIT(0)	/*PCI ASPM */
#define	RT_RF_OFF_LEVL_CLK_REQ		BIT(1)	/*PCI clock request */
#define	RT_RF_OFF_LEVL_PCI_D3		BIT(2)	/*PCI D3 mode */
/*NIC halt, re-initialize hw parameters*/
#define	RT_RF_OFF_LEVL_HALT_NIC		BIT(3)
#define	RT_RF_OFF_LEVL_FREE_FW		BIT(4)	/*FW free, re-download the FW */
#define	RT_RF_OFF_LEVL_FW_32K		BIT(5)	/*FW in 32k */
/*Always enable ASPM and Clock Req in initialization.*/
#define	RT_RF_PS_LEVEL_ALWAYS_ASPM	BIT(6)
/* no matter RFOFF or SLEEP we set PS_ASPM_LEVL*/
#define	RT_PS_LEVEL_ASPM		BIT(7)
/*When LPS is on, disable 2R if no packet is received or transmittd.*/
#define	RT_RF_LPS_DISALBE_2R		BIT(30)
#define	RT_RF_LPS_LEVEL_ASPM		BIT(31)	/*LPS with ASPM */
#define	RT_IN_PS_LEVEL(ppsc, _ps_flg)		\
	((ppsc->cur_ps_level & _ps_flg) ? true : false)
#define	RT_CLEAR_PS_LEVEL(ppsc, _ps_flg)	\
	(ppsc->cur_ps_level &= (~(_ps_flg)))
#define	RT_SET_PS_LEVEL(ppsc, _ps_flg)		\
	(ppsc->cur_ps_level |= _ps_flg)

#define container_of_dwork_rtl(x, y, z) \
	container_of(container_of(x, struct delayed_work, work), y, z)

#define FILL_OCTET_STRING(_os, _octet, _len)	\
		(_os).octet = (u8 *)(_octet);		\
		(_os).length = (_len);

#define CP_MACADDR(des, src)	\
	((des)[0] = (src)[0], (des)[1] = (src)[1],\
	(des)[2] = (src)[2], (des)[3] = (src)[3],\
	(des)[4] = (src)[4], (des)[5] = (src)[5])

static inline u8 rtl_read_byte(struct rtl_priv *rtlpriv, u32 addr)
{
	return rtlpriv->io.read8_sync(rtlpriv, addr);
}

static inline u16 rtl_read_word(struct rtl_priv *rtlpriv, u32 addr)
{
	return rtlpriv->io.read16_sync(rtlpriv, addr);
}

static inline u32 rtl_read_dword(struct rtl_priv *rtlpriv, u32 addr)
{
	return rtlpriv->io.read32_sync(rtlpriv, addr);
}

static inline void rtl_write_byte(struct rtl_priv *rtlpriv, u32 addr, u8 val8)
{
	rtlpriv->io.write8_async(rtlpriv, addr, val8);

	if (rtlpriv->cfg->write_readback)
		rtlpriv->io.read8_sync(rtlpriv, addr);
}

static inline void rtl_write_word(struct rtl_priv *rtlpriv, u32 addr, u16 val16)
{
	rtlpriv->io.write16_async(rtlpriv, addr, val16);

	if (rtlpriv->cfg->write_readback)
		rtlpriv->io.read16_sync(rtlpriv, addr);
}

static inline void rtl_write_dword(struct rtl_priv *rtlpriv,
				   u32 addr, u32 val32)
{
	rtlpriv->io.write32_async(rtlpriv, addr, val32);

	if (rtlpriv->cfg->write_readback)
		rtlpriv->io.read32_sync(rtlpriv, addr);
}

static inline u32 rtl_get_bbreg(struct ieee80211_hw *hw,
				u32 regaddr, u32 bitmask)
{
	struct rtl_priv *rtlpriv = hw->priv;

	return rtlpriv->cfg->ops->get_bbreg(hw, regaddr, bitmask);
}

static inline void rtl_set_bbreg(struct ieee80211_hw *hw, u32 regaddr,
				 u32 bitmask, u32 data)
{
	struct rtl_priv *rtlpriv = hw->priv;

	rtlpriv->cfg->ops->set_bbreg(hw, regaddr, bitmask, data);
}

static inline u32 rtl_get_rfreg(struct ieee80211_hw *hw,
				enum radio_path rfpath, u32 regaddr,
				u32 bitmask)
{
	struct rtl_priv *rtlpriv = hw->priv;

	return rtlpriv->cfg->ops->get_rfreg(hw, rfpath, regaddr, bitmask);
}

static inline void rtl_set_rfreg(struct ieee80211_hw *hw,
				 enum radio_path rfpath, u32 regaddr,
				 u32 bitmask, u32 data)
{
	struct rtl_priv *rtlpriv = hw->priv;

	rtlpriv->cfg->ops->set_rfreg(hw, rfpath, regaddr, bitmask, data);
}

static inline bool is_hal_stop(struct rtl_hal *rtlhal)
{
	return (_HAL_STATE_STOP == rtlhal->state);
}

static inline void set_hal_start(struct rtl_hal *rtlhal)
{
	rtlhal->state = _HAL_STATE_START;
}

static inline void set_hal_stop(struct rtl_hal *rtlhal)
{
	rtlhal->state = _HAL_STATE_STOP;
}

static inline u8 get_rf_type(struct rtl_phy *rtlphy)
{
	return rtlphy->rf_type;
}

static inline struct ieee80211_hdr *rtl_get_hdr(struct sk_buff *skb)
{
	return (struct ieee80211_hdr *)(skb->data);
}

static inline __le16 rtl_get_fc(struct sk_buff *skb)
{
	return rtl_get_hdr(skb)->frame_control;
}

static inline u16 rtl_get_tid_h(struct ieee80211_hdr *hdr)
{
	return (ieee80211_get_qos_ctl(hdr))[0] & IEEE80211_QOS_CTL_TID_MASK;
}

static inline u16 rtl_get_tid(struct sk_buff *skb)
{
	return rtl_get_tid_h(rtl_get_hdr(skb));
}

static inline struct ieee80211_sta *get_sta(struct ieee80211_hw *hw,
					    struct ieee80211_vif *vif,
					    const u8 *bssid)
{
	return ieee80211_find_sta(vif, bssid);
}

static inline struct ieee80211_sta *rtl_find_sta(struct ieee80211_hw *hw,
		u8 *mac_addr)
{
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	return ieee80211_find_sta(mac->vif, mac_addr);
}

#endif
	union {
		struct {	/* data buffer pointer for USB reads */
			__le32 *usb_data;
			int usb_data_index;
			bool initialized;
		};
		struct {	/* section for 8723ae */
			bool reg_init;	/* true if regs saved */
			u32 reg_874;
			u32 reg_c70;
			u32 reg_85c;
			u32 reg_a74;
			bool bt_operation_on;
		};
	};
	bool enter_ps;	/* true when entering PS */

	/*This must be the last item so
	   that it points to the data allocated
	   beyond  this structure like:
	   rtl_pci_priv or rtl_usb_priv */
	u8 priv[0];
};

#define rtl_priv(hw)		(((struct rtl_priv *)(hw)->priv))
#define rtl_mac(rtlpriv)	(&((rtlpriv)->mac80211))
#define rtl_hal(rtlpriv)	(&((rtlpriv)->rtlhal))
#define rtl_efuse(rtlpriv)	(&((rtlpriv)->efuse))
#define rtl_psc(rtlpriv)	(&((rtlpriv)->psc))


/***************************************
    Bluetooth Co-existence Related
****************************************/

enum bt_ant_num {
	ANT_X2 = 0,
	ANT_X1 = 1,
};

enum bt_co_type {
	BT_2WIRE = 0,
	BT_ISSC_3WIRE = 1,
	BT_ACCEL = 2,
	BT_CSR_BC4 = 3,
	BT_CSR_BC8 = 4,
	BT_RTL8756 = 5,
	BT_RTL8723A = 6,
};

enum bt_cur_state {
	BT_OFF = 0,
	BT_ON = 1,
};

enum bt_service_type {
	BT_SCO = 0,
	BT_A2DP = 1,
	BT_HID = 2,
	BT_HID_IDLE = 3,
	BT_SCAN = 4,
	BT_IDLE = 5,
	BT_OTHER_ACTION = 6,
	BT_BUSY = 7,
	BT_OTHERBUSY = 8,
	BT_PAN = 9,
};

enum bt_radio_shared {
	BT_RADIO_SHARED = 0,
	BT_RADIO_INDIVIDUAL = 1,
};

struct bt_coexist_info {

	/* EEPROM BT info. */
	u8 eeprom_bt_coexist;
	u8 eeprom_bt_type;
	u8 eeprom_bt_ant_num;
	u8 eeprom_bt_ant_isol;
	u8 eeprom_bt_radio_shared;

	u8 bt_coexistence;
	u8 bt_ant_num;
	u8 bt_coexist_type;
	u8 bt_state;
	u8 bt_cur_state;	/* 0:on, 1:off */
	u8 bt_ant_isolation;	/* 0:good, 1:bad */
	u8 bt_pape_ctrl;	/* 0:SW, 1:SW/HW dynamic */
	u8 bt_service;
	u8 bt_radio_shared_type;
	u8 bt_rfreg_origin_1e;
	u8 bt_rfreg_origin_1f;
	u8 bt_rssi_state;
	u32 ratio_tx;
	u32 ratio_pri;
	u32 bt_edca_ul;
	u32 bt_edca_dl;

	bool init_set;
	bool bt_busy_traffic;
	bool bt_traffic_mode_set;
	bool bt_non_traffic_mode_set;

	bool fw_coexist_all_off;
	bool sw_coexist_all_off;
	bool hw_coexist_all_off;
	u32 cstate;
	u32 previous_state;
	u32 cstate_h;
	u32 previous_state_h;

	u8 bt_pre_rssi_state;
	u8 bt_pre_rssi_state1;

	u8 reg_bt_iso;
	u8 reg_bt_sco;
	bool balance_on;
	u8 bt_active_zero_cnt;
	bool cur_bt_disabled;
	bool pre_bt_disabled;

	u8 bt_profile_case;
	u8 bt_profile_action;
	bool bt_busy;
	bool hold_for_bt_operation;
	u8 lps_counter;
};


/****************************************
	mem access macro define start
	Call endian free function when
	1. Read/write packet content.
	2. Before write integer to IO.
	3. After read integer from IO.
****************************************/
/* Convert little data endian to host ordering */
#define EF1BYTE(_val)		\
	((u8)(_val))
#define EF2BYTE(_val)		\
	(le16_to_cpu(_val))
#define EF4BYTE(_val)		\
	(le32_to_cpu(_val))

/* Read data from memory */
#define READEF1BYTE(_ptr)	\
	EF1BYTE(*((u8 *)(_ptr)))
/* Read le16 data from memory and convert to host ordering */
#define READEF2BYTE(_ptr)	\
	EF2BYTE(*(_ptr))
#define READEF4BYTE(_ptr)	\
	EF4BYTE(*(_ptr))

/* Write data to memory */
#define WRITEEF1BYTE(_ptr, _val)	\
	(*((u8 *)(_ptr))) = EF1BYTE(_val)
/* Write le16 data to memory in host ordering */
#define WRITEEF2BYTE(_ptr, _val)	\
	(*((u16 *)(_ptr))) = EF2BYTE(_val)
#define WRITEEF4BYTE(_ptr, _val)	\
	(*((u32 *)(_ptr))) = EF2BYTE(_val)

/* Create a bit mask
 * Examples:
 * BIT_LEN_MASK_32(0) => 0x00000000
 * BIT_LEN_MASK_32(1) => 0x00000001
 * BIT_LEN_MASK_32(2) => 0x00000003
 * BIT_LEN_MASK_32(32) => 0xFFFFFFFF
 */
#define BIT_LEN_MASK_32(__bitlen)	 \
	(0xFFFFFFFF >> (32 - (__bitlen)))
#define BIT_LEN_MASK_16(__bitlen)	 \
	(0xFFFF >> (16 - (__bitlen)))
#define BIT_LEN_MASK_8(__bitlen) \
	(0xFF >> (8 - (__bitlen)))

/* Create an offset bit mask
 * Examples:
 * BIT_OFFSET_LEN_MASK_32(0, 2) => 0x00000003
 * BIT_OFFSET_LEN_MASK_32(16, 2) => 0x00030000
 */
#define BIT_OFFSET_LEN_MASK_32(__bitoffset, __bitlen) \
	(BIT_LEN_MASK_32(__bitlen) << (__bitoffset))
#define BIT_OFFSET_LEN_MASK_16(__bitoffset, __bitlen) \
	(BIT_LEN_MASK_16(__bitlen) << (__bitoffset))
#define BIT_OFFSET_LEN_MASK_8(__bitoffset, __bitlen) \
	(BIT_LEN_MASK_8(__bitlen) << (__bitoffset))

/*Description:
 * Return 4-byte value in host byte ordering from
 * 4-byte pointer in little-endian system.
 */
#define LE_P4BYTE_TO_HOST_4BYTE(__pstart) \
	(EF4BYTE(*((__le32 *)(__pstart))))
#define LE_P2BYTE_TO_HOST_2BYTE(__pstart) \
	(EF2BYTE(*((__le16 *)(__pstart))))
#define LE_P1BYTE_TO_HOST_1BYTE(__pstart) \
	(EF1BYTE(*((u8 *)(__pstart))))

/*Description:
Translate subfield (continuous bits in little-endian) of 4-byte
value to host byte ordering.*/
#define LE_BITS_TO_4BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		(LE_P4BYTE_TO_HOST_4BYTE(__pstart) >> (__bitoffset))  & \
		BIT_LEN_MASK_32(__bitlen) \
	)
#define LE_BITS_TO_2BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		(LE_P2BYTE_TO_HOST_2BYTE(__pstart) >> (__bitoffset)) & \
		BIT_LEN_MASK_16(__bitlen) \
	)
#define LE_BITS_TO_1BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		(LE_P1BYTE_TO_HOST_1BYTE(__pstart) >> (__bitoffset)) & \
		BIT_LEN_MASK_8(__bitlen) \
	)

/* Description:
 * Mask subfield (continuous bits in little-endian) of 4-byte value
 * and return the result in 4-byte value in host byte ordering.
 */
#define LE_BITS_CLEARED_TO_4BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		LE_P4BYTE_TO_HOST_4BYTE(__pstart)  & \
		(~BIT_OFFSET_LEN_MASK_32(__bitoffset, __bitlen)) \
	)
#define LE_BITS_CLEARED_TO_2BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		LE_P2BYTE_TO_HOST_2BYTE(__pstart) & \
		(~BIT_OFFSET_LEN_MASK_16(__bitoffset, __bitlen)) \
	)
#define LE_BITS_CLEARED_TO_1BYTE(__pstart, __bitoffset, __bitlen) \
	( \
		LE_P1BYTE_TO_HOST_1BYTE(__pstart) & \
		(~BIT_OFFSET_LEN_MASK_8(__bitoffset, __bitlen)) \
	)

/* Description:
 * Set subfield of little-endian 4-byte value to specified value.
 */
#define SET_BITS_TO_LE_4BYTE(__pstart, __bitoffset, __bitlen, __val) \
	*((u32 *)(__pstart)) = \
	( \
		LE_BITS_CLEARED_TO_4BYTE(__pstart, __bitoffset, __bitlen) | \
		((((u32)__val) & BIT_LEN_MASK_32(__bitlen)) << (__bitoffset)) \
	);
#define SET_BITS_TO_LE_2BYTE(__pstart, __bitoffset, __bitlen, __val) \
	*((u16 *)(__pstart)) = \
	( \
		LE_BITS_CLEARED_TO_2BYTE(__pstart, __bitoffset, __bitlen) | \
		((((u16)__val) & BIT_LEN_MASK_16(__bitlen)) << (__bitoffset)) \
	);
#define SET_BITS_TO_LE_1BYTE(__pstart, __bitoffset, __bitlen, __val) \
	*((u8 *)(__pstart)) = EF1BYTE \
	( \
		LE_BITS_CLEARED_TO_1BYTE(__pstart, __bitoffset, __bitlen) | \
		((((u8)__val) & BIT_LEN_MASK_8(__bitlen)) << (__bitoffset)) \
	);

#define	N_BYTE_ALIGMENT(__value, __aligment) ((__aligment == 1) ? \
	(__value) : (((__value + __aligment - 1) / __aligment) * __aligment))

/****************************************
	mem access macro define end
****************************************/

#define byte(x, n) ((x >> (8 * n)) & 0xff)

#define packet_get_type(_packet) (EF1BYTE((_packet).octet[0]) & 0xFC)
#define RTL_WATCH_DOG_TIME	2000
#define MSECS(t)		msecs_to_jiffies(t)
#define WLAN_FC_GET_VERS(fc)	(le16_to_cpu(fc) & IEEE80211_FCTL_VERS)
#define WLAN_FC_GET_TYPE(fc)	(le16_to_cpu(fc) & IEEE80211_FCTL_FTYPE)
#define WLAN_FC_GET_STYPE(fc)	(le16_to_cpu(fc) & IEEE80211_FCTL_STYPE)
#define WLAN_FC_MORE_DATA(fc)	(le16_to_cpu(fc) & IEEE80211_FCTL_MOREDATA)
#define rtl_dm(rtlpriv)		(&((rtlpriv)->dm))

#define	RT_RF_OFF_LEVL_ASPM		BIT(0)	/*PCI ASPM */
#define	RT_RF_OFF_LEVL_CLK_REQ		BIT(1)	/*PCI clock request */
#define	RT_RF_OFF_LEVL_PCI_D3		BIT(2)	/*PCI D3 mode */
/*NIC halt, re-initialize hw parameters*/
#define	RT_RF_OFF_LEVL_HALT_NIC		BIT(3)
#define	RT_RF_OFF_LEVL_FREE_FW		BIT(4)	/*FW free, re-download the FW */
#define	RT_RF_OFF_LEVL_FW_32K		BIT(5)	/*FW in 32k */
/*Always enable ASPM and Clock Req in initialization.*/
#define	RT_RF_PS_LEVEL_ALWAYS_ASPM	BIT(6)
/* no matter RFOFF or SLEEP we set PS_ASPM_LEVL*/
#define	RT_PS_LEVEL_ASPM		BIT(7)
/*When LPS is on, disable 2R if no packet is received or transmittd.*/
#define	RT_RF_LPS_DISALBE_2R		BIT(30)
#define	RT_RF_LPS_LEVEL_ASPM		BIT(31)	/*LPS with ASPM */
#define	RT_IN_PS_LEVEL(ppsc, _ps_flg)		\
	((ppsc->cur_ps_level & _ps_flg) ? true : false)
#define	RT_CLEAR_PS_LEVEL(ppsc, _ps_flg)	\
	(ppsc->cur_ps_level &= (~(_ps_flg)))
#define	RT_SET_PS_LEVEL(ppsc, _ps_flg)		\
	(ppsc->cur_ps_level |= _ps_flg)

#define container_of_dwork_rtl(x, y, z) \
	container_of(container_of(x, struct delayed_work, work), y, z)

#define FILL_OCTET_STRING(_os, _octet, _len)	\
		(_os).octet = (u8 *)(_octet);		\
		(_os).length = (_len);

#define CP_MACADDR(des, src)	\
	((des)[0] = (src)[0], (des)[1] = (src)[1],\
	(des)[2] = (src)[2], (des)[3] = (src)[3],\
	(des)[4] = (src)[4], (des)[5] = (src)[5])

static inline u8 rtl_read_byte(struct rtl_priv *rtlpriv, u32 addr)
{
	return rtlpriv->io.read8_sync(rtlpriv, addr);
}

static inline u16 rtl_read_word(struct rtl_priv *rtlpriv, u32 addr)
{
	return rtlpriv->io.read16_sync(rtlpriv, addr);
}

static inline u32 rtl_read_dword(struct rtl_priv *rtlpriv, u32 addr)
{
	return rtlpriv->io.read32_sync(rtlpriv, addr);
}

static inline void rtl_write_byte(struct rtl_priv *rtlpriv, u32 addr, u8 val8)
{
	rtlpriv->io.write8_async(rtlpriv, addr, val8);

	if (rtlpriv->cfg->write_readback)
		rtlpriv->io.read8_sync(rtlpriv, addr);
}

static inline void rtl_write_word(struct rtl_priv *rtlpriv, u32 addr, u16 val16)
{
	rtlpriv->io.write16_async(rtlpriv, addr, val16);

	if (rtlpriv->cfg->write_readback)
		rtlpriv->io.read16_sync(rtlpriv, addr);
}

static inline void rtl_write_dword(struct rtl_priv *rtlpriv,
				   u32 addr, u32 val32)
{
	rtlpriv->io.write32_async(rtlpriv, addr, val32);

	if (rtlpriv->cfg->write_readback)
		rtlpriv->io.read32_sync(rtlpriv, addr);
}

static inline u32 rtl_get_bbreg(struct ieee80211_hw *hw,
				u32 regaddr, u32 bitmask)
{
	struct rtl_priv *rtlpriv = hw->priv;

	return rtlpriv->cfg->ops->get_bbreg(hw, regaddr, bitmask);
}

static inline void rtl_set_bbreg(struct ieee80211_hw *hw, u32 regaddr,
				 u32 bitmask, u32 data)
{
	struct rtl_priv *rtlpriv = hw->priv;

	rtlpriv->cfg->ops->set_bbreg(hw, regaddr, bitmask, data);
}

static inline u32 rtl_get_rfreg(struct ieee80211_hw *hw,
				enum radio_path rfpath, u32 regaddr,
				u32 bitmask)
{
	struct rtl_priv *rtlpriv = hw->priv;

	return rtlpriv->cfg->ops->get_rfreg(hw, rfpath, regaddr, bitmask);
}

static inline void rtl_set_rfreg(struct ieee80211_hw *hw,
				 enum radio_path rfpath, u32 regaddr,
				 u32 bitmask, u32 data)
{
	struct rtl_priv *rtlpriv = hw->priv;

	rtlpriv->cfg->ops->set_rfreg(hw, rfpath, regaddr, bitmask, data);
}

static inline bool is_hal_stop(struct rtl_hal *rtlhal)
{
	return (_HAL_STATE_STOP == rtlhal->state);
}

static inline void set_hal_start(struct rtl_hal *rtlhal)
{
	rtlhal->state = _HAL_STATE_START;
}

static inline void set_hal_stop(struct rtl_hal *rtlhal)
{
	rtlhal->state = _HAL_STATE_STOP;
}

static inline u8 get_rf_type(struct rtl_phy *rtlphy)
{
	return rtlphy->rf_type;
}

static inline struct ieee80211_hdr *rtl_get_hdr(struct sk_buff *skb)
{
	return (struct ieee80211_hdr *)(skb->data);
}

static inline __le16 rtl_get_fc(struct sk_buff *skb)
{
	return rtl_get_hdr(skb)->frame_control;
}

static inline u16 rtl_get_tid_h(struct ieee80211_hdr *hdr)
{
	return (ieee80211_get_qos_ctl(hdr))[0] & IEEE80211_QOS_CTL_TID_MASK;
}

static inline u16 rtl_get_tid(struct sk_buff *skb)
{
	return rtl_get_tid_h(rtl_get_hdr(skb));
}

static inline struct ieee80211_sta *get_sta(struct ieee80211_hw *hw,
					    struct ieee80211_vif *vif,
					    const u8 *bssid)
{
	return ieee80211_find_sta(vif, bssid);
}

static inline struct ieee80211_sta *rtl_find_sta(struct ieee80211_hw *hw,
		u8 *mac_addr)
{
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	return ieee80211_find_sta(mac->vif, mac_addr);
}

#endif