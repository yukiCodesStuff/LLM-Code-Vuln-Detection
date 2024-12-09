enum iwl_nvm_type {
	IWL_NVM,
	IWL_NVM_EXT,
	IWL_NVM_SDP,
};

/*
 * This is the threshold value of plcp error rate per 100mSecs.  It is
 * used to set and check for the validity of plcp_delta.
 */
#define IWL_MAX_PLCP_ERR_THRESHOLD_MIN		1
#define IWL_MAX_PLCP_ERR_THRESHOLD_DEF		50
#define IWL_MAX_PLCP_ERR_LONG_THRESHOLD_DEF	100
#define IWL_MAX_PLCP_ERR_EXT_LONG_THRESHOLD_DEF	200
#define IWL_MAX_PLCP_ERR_THRESHOLD_MAX		255
#define IWL_MAX_PLCP_ERR_THRESHOLD_DISABLE	0

/* TX queue watchdog timeouts in mSecs */
#define IWL_WATCHDOG_DISABLED	0
#define IWL_DEF_WD_TIMEOUT	2500
#define IWL_LONG_WD_TIMEOUT	10000
#define IWL_MAX_WD_TIMEOUT	120000

#define IWL_DEFAULT_MAX_TX_POWER 22
#define IWL_TX_CSUM_NETIF_FLAGS (NETIF_F_IPV6_CSUM | NETIF_F_IP_CSUM |\
				 NETIF_F_TSO | NETIF_F_TSO6)

/* Antenna presence definitions */
#define	ANT_NONE	0x0
#define	ANT_INVALID	0xff
#define	ANT_A		BIT(0)
#define	ANT_B		BIT(1)
#define ANT_C		BIT(2)
#define	ANT_AB		(ANT_A | ANT_B)
#define	ANT_AC		(ANT_A | ANT_C)
#define ANT_BC		(ANT_B | ANT_C)
#define ANT_ABC		(ANT_A | ANT_B | ANT_C)
#define MAX_ANT_NUM 3


static inline u8 num_of_ant(u8 mask)
{
	return  !!((mask) & ANT_A) +
		!!((mask) & ANT_B) +
		!!((mask) & ANT_C);
}
struct iwl_cfg {
	struct iwl_cfg_trans_params trans;
	/* params specific to an individual device within a device family */
	const char *name;
	const char *fw_name_pre;
	/* params likely to change within a device family */
	const struct iwl_ht_params *ht_params;
	const struct iwl_eeprom_params *eeprom_params;
	const struct iwl_pwr_tx_backoff *pwr_tx_backoffs;
	const char *default_nvm_file_C_step;
	const struct iwl_tt_params *thermal_params;
	enum iwl_led_mode led_mode;
	enum iwl_nvm_type nvm_type;
	u32 max_data_size;
	u32 max_inst_size;
	netdev_features_t features;
	u32 dccm_offset;
	u32 dccm_len;
	u32 dccm2_offset;
	u32 dccm2_len;
	u32 smem_offset;
	u32 smem_len;
	u16 nvm_ver;
	u16 nvm_calib_ver;
	u32 rx_with_siso_diversity:1,
	    tx_with_siso_diversity:1,
	    bt_shared_single_ant:1,
	    internal_wimax_coex:1,
	    host_interrupt_operation_mode:1,
	    high_temp:1,
	    mac_addr_from_csr:10,
	    lp_xtal_workaround:1,
	    disable_dummy_notification:1,
	    apmg_not_supported:1,
	    vht_mu_mimo_supported:1,
	    cdb:1,
	    dbgc_supported:1,
	    uhb_supported:1;
	u8 valid_tx_ant;
	u8 valid_rx_ant;
	u8 non_shared_ant;
	u8 nvm_hw_section_num;
	u8 max_tx_agg_size;
	u8 ucode_api_max;
	u8 ucode_api_min;
	u16 num_rbds;
	u32 min_umac_error_event_table;
	u32 d3_debug_data_base_addr;
	u32 d3_debug_data_length;
	u32 min_txq_size;
	u32 gp2_reg_addr;
	u32 min_256_ba_txq_size;
	const struct iwl_fw_mon_regs mon_dram_regs;
	const struct iwl_fw_mon_regs mon_smem_regs;
};

#define IWL_CFG_ANY (~0)

#define IWL_CFG_MAC_TYPE_PU		0x31
#define IWL_CFG_MAC_TYPE_PNJ		0x32
#define IWL_CFG_MAC_TYPE_TH		0x32
#define IWL_CFG_MAC_TYPE_QU		0x33
#define IWL_CFG_MAC_TYPE_QUZ		0x35
#define IWL_CFG_MAC_TYPE_QNJ		0x36
#define IWL_CFG_MAC_TYPE_SO		0x37
#define IWL_CFG_MAC_TYPE_SNJ		0x42
#define IWL_CFG_MAC_TYPE_SOF		0x43
#define IWL_CFG_MAC_TYPE_MA		0x44
#define IWL_CFG_MAC_TYPE_BZ		0x46

#define IWL_CFG_RF_TYPE_TH		0x105
#define IWL_CFG_RF_TYPE_TH1		0x108
#define IWL_CFG_RF_TYPE_JF2		0x105
#define IWL_CFG_RF_TYPE_JF1		0x108
#define IWL_CFG_RF_TYPE_HR2		0x10A
#define IWL_CFG_RF_TYPE_HR1		0x10C
#define IWL_CFG_RF_TYPE_GF		0x10D
#define IWL_CFG_RF_TYPE_MR		0x110
#define IWL_CFG_RF_TYPE_FM		0x112

#define IWL_CFG_RF_ID_TH		0x1
#define IWL_CFG_RF_ID_TH1		0x1
#define IWL_CFG_RF_ID_JF		0x3
#define IWL_CFG_RF_ID_JF1		0x6
#define IWL_CFG_RF_ID_JF1_DIV		0xA
#define IWL_CFG_RF_ID_HR		0x7
#define IWL_CFG_RF_ID_HR1		0x4

#define IWL_CFG_NO_160			0x1
#define IWL_CFG_160			0x0

#define IWL_CFG_CORES_BT		0x0
#define IWL_CFG_CORES_BT_GNSS		0x5

#define IWL_CFG_NO_CDB			0x0
#define IWL_CFG_CDB			0x1

#define IWL_SUBDEVICE_RF_ID(subdevice)	((u16)((subdevice) & 0x00F0) >> 4)
#define IWL_SUBDEVICE_NO_160(subdevice)	((u16)((subdevice) & 0x0200) >> 9)
#define IWL_SUBDEVICE_CORES(subdevice)	((u16)((subdevice) & 0x1C00) >> 10)

struct iwl_dev_info {
	u16 device;
	u16 subdevice;
	u16 mac_type;
	u16 rf_type;
	u8 mac_step;
	u8 rf_id;
	u8 no_160;
	u8 cores;
	u8 cdb;
	const struct iwl_cfg *cfg;
	const char *name;
};

/*
 * This list declares the config structures for all devices.
 */
extern const struct iwl_cfg_trans_params iwl9000_trans_cfg;
extern const struct iwl_cfg_trans_params iwl9560_trans_cfg;
extern const struct iwl_cfg_trans_params iwl9560_long_latency_trans_cfg;
extern const struct iwl_cfg_trans_params iwl9560_shared_clk_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_qnj_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_qu_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_qu_medium_latency_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_qu_long_latency_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_ax200_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_snj_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_so_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_so_long_latency_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_ma_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_bz_trans_cfg;
extern const char iwl9162_name[];
extern const char iwl9260_name[];
extern const char iwl9260_1_name[];
extern const char iwl9270_name[];
extern const char iwl9461_name[];
extern const char iwl9462_name[];
extern const char iwl9560_name[];
extern const char iwl9162_160_name[];
extern const char iwl9260_160_name[];
extern const char iwl9270_160_name[];
extern const char iwl9461_160_name[];
extern const char iwl9462_160_name[];
extern const char iwl9560_160_name[];
extern const char iwl9260_killer_1550_name[];
extern const char iwl9560_killer_1550i_name[];
extern const char iwl9560_killer_1550s_name[];
extern const char iwl_ax200_name[];
extern const char iwl_ax203_name[];
extern const char iwl_ax201_name[];
extern const char iwl_ax101_name[];
extern const char iwl_ax200_killer_1650w_name[];
extern const char iwl_ax200_killer_1650x_name[];
extern const char iwl_ax201_killer_1650s_name[];
extern const char iwl_ax201_killer_1650i_name[];
extern const char iwl_ax210_killer_1675w_name[];
extern const char iwl_ax210_killer_1675x_name[];
extern const char iwl9560_killer_1550i_160_name[];
extern const char iwl9560_killer_1550s_160_name[];
extern const char iwl_ax211_name[];
extern const char iwl_ax221_name[];
extern const char iwl_ax231_name[];
extern const char iwl_ax411_name[];
extern const char iwl_bz_name[];
#if IS_ENABLED(CONFIG_IWLDVM)
extern const struct iwl_cfg iwl5300_agn_cfg;
extern const struct iwl_cfg iwl5100_agn_cfg;
extern const struct iwl_cfg iwl5350_agn_cfg;
extern const struct iwl_cfg iwl5100_bgn_cfg;
extern const struct iwl_cfg iwl5100_abg_cfg;
extern const struct iwl_cfg iwl5150_agn_cfg;
extern const struct iwl_cfg iwl5150_abg_cfg;
extern const struct iwl_cfg iwl6005_2agn_cfg;
extern const struct iwl_cfg iwl6005_2abg_cfg;
extern const struct iwl_cfg iwl6005_2bg_cfg;
extern const struct iwl_cfg iwl6005_2agn_sff_cfg;
extern const struct iwl_cfg iwl6005_2agn_d_cfg;
extern const struct iwl_cfg iwl6005_2agn_mow1_cfg;
extern const struct iwl_cfg iwl6005_2agn_mow2_cfg;
extern const struct iwl_cfg iwl1030_bgn_cfg;
extern const struct iwl_cfg iwl1030_bg_cfg;
extern const struct iwl_cfg iwl6030_2agn_cfg;
extern const struct iwl_cfg iwl6030_2abg_cfg;
extern const struct iwl_cfg iwl6030_2bgn_cfg;
extern const struct iwl_cfg iwl6030_2bg_cfg;
extern const struct iwl_cfg iwl6000i_2agn_cfg;
extern const struct iwl_cfg iwl6000i_2abg_cfg;
extern const struct iwl_cfg iwl6000i_2bg_cfg;
extern const struct iwl_cfg iwl6000_3agn_cfg;
extern const struct iwl_cfg iwl6050_2agn_cfg;
extern const struct iwl_cfg iwl6050_2abg_cfg;
extern const struct iwl_cfg iwl6150_bgn_cfg;
extern const struct iwl_cfg iwl6150_bg_cfg;
extern const struct iwl_cfg iwl1000_bgn_cfg;
extern const struct iwl_cfg iwl1000_bg_cfg;
extern const struct iwl_cfg iwl100_bgn_cfg;
extern const struct iwl_cfg iwl100_bg_cfg;
extern const struct iwl_cfg iwl130_bgn_cfg;
extern const struct iwl_cfg iwl130_bg_cfg;
extern const struct iwl_cfg iwl2000_2bgn_cfg;
extern const struct iwl_cfg iwl2000_2bgn_d_cfg;
extern const struct iwl_cfg iwl2030_2bgn_cfg;
extern const struct iwl_cfg iwl6035_2agn_cfg;
extern const struct iwl_cfg iwl6035_2agn_sff_cfg;
extern const struct iwl_cfg iwl105_bgn_cfg;
extern const struct iwl_cfg iwl105_bgn_d_cfg;
extern const struct iwl_cfg iwl135_bgn_cfg;
#endif /* CONFIG_IWLDVM */
#if IS_ENABLED(CONFIG_IWLMVM)
extern const struct iwl_cfg iwl7260_2ac_cfg;
extern const struct iwl_cfg iwl7260_2ac_cfg_high_temp;
extern const struct iwl_cfg iwl7260_2n_cfg;
extern const struct iwl_cfg iwl7260_n_cfg;
extern const struct iwl_cfg iwl3160_2ac_cfg;
extern const struct iwl_cfg iwl3160_2n_cfg;
extern const struct iwl_cfg iwl3160_n_cfg;
extern const struct iwl_cfg iwl3165_2ac_cfg;
extern const struct iwl_cfg iwl3168_2ac_cfg;
extern const struct iwl_cfg iwl7265_2ac_cfg;
extern const struct iwl_cfg iwl7265_2n_cfg;
extern const struct iwl_cfg iwl7265_n_cfg;
extern const struct iwl_cfg iwl7265d_2ac_cfg;
extern const struct iwl_cfg iwl7265d_2n_cfg;
extern const struct iwl_cfg iwl7265d_n_cfg;
extern const struct iwl_cfg iwl8260_2n_cfg;
extern const struct iwl_cfg iwl8260_2ac_cfg;
extern const struct iwl_cfg iwl8265_2ac_cfg;
extern const struct iwl_cfg iwl8275_2ac_cfg;
extern const struct iwl_cfg iwl4165_2ac_cfg;
extern const struct iwl_cfg iwl9260_2ac_cfg;
extern const struct iwl_cfg iwl9560_qu_b0_jf_b0_cfg;
extern const struct iwl_cfg iwl9560_qu_c0_jf_b0_cfg;
extern const struct iwl_cfg iwl9560_quz_a0_jf_b0_cfg;
extern const struct iwl_cfg iwl9560_qnj_b0_jf_b0_cfg;
extern const struct iwl_cfg iwl9560_2ac_cfg_soc;
extern const struct iwl_cfg iwl_qu_b0_hr1_b0;
extern const struct iwl_cfg iwl_qu_c0_hr1_b0;
extern const struct iwl_cfg iwl_quz_a0_hr1_b0;
extern const struct iwl_cfg iwl_qu_b0_hr_b0;
extern const struct iwl_cfg iwl_qu_c0_hr_b0;
extern const struct iwl_cfg iwl_ax200_cfg_cc;
extern const struct iwl_cfg iwl_ax201_cfg_qu_hr;
extern const struct iwl_cfg iwl_ax201_cfg_qu_c0_hr_b0;
extern const struct iwl_cfg iwl_ax201_cfg_quz_hr;
extern const struct iwl_cfg iwl_ax1650i_cfg_quz_hr;
extern const struct iwl_cfg iwl_ax1650s_cfg_quz_hr;
extern const struct iwl_cfg killer1650s_2ax_cfg_qu_b0_hr_b0;
extern const struct iwl_cfg killer1650i_2ax_cfg_qu_b0_hr_b0;
extern const struct iwl_cfg killer1650s_2ax_cfg_qu_c0_hr_b0;
extern const struct iwl_cfg killer1650i_2ax_cfg_qu_c0_hr_b0;
extern const struct iwl_cfg killer1650x_2ax_cfg;
extern const struct iwl_cfg killer1650w_2ax_cfg;
extern const struct iwl_cfg iwl_qnj_b0_hr_b0_cfg;
extern const struct iwl_cfg iwlax210_2ax_cfg_so_jf_b0;
extern const struct iwl_cfg iwlax210_2ax_cfg_so_hr_a0;
extern const struct iwl_cfg iwlax211_2ax_cfg_so_gf_a0;
extern const struct iwl_cfg iwlax211_2ax_cfg_so_gf_a0_long;
extern const struct iwl_cfg iwlax210_2ax_cfg_ty_gf_a0;
extern const struct iwl_cfg iwlax411_2ax_cfg_so_gf4_a0;
extern const struct iwl_cfg iwlax411_2ax_cfg_so_gf4_a0_long;
extern const struct iwl_cfg iwlax411_2ax_cfg_sosnj_gf4_a0;
extern const struct iwl_cfg iwlax211_cfg_snj_gf_a0;
extern const struct iwl_cfg iwl_cfg_snj_hr_b0;
extern const struct iwl_cfg iwl_cfg_snj_a0_jf_b0;
extern const struct iwl_cfg iwl_cfg_ma_a0_hr_b0;
extern const struct iwl_cfg iwl_cfg_ma_a0_gf_a0;
extern const struct iwl_cfg iwl_cfg_ma_a0_gf4_a0;
extern const struct iwl_cfg iwl_cfg_ma_a0_mr_a0;
extern const struct iwl_cfg iwl_cfg_ma_a0_fm_a0;
extern const struct iwl_cfg iwl_cfg_snj_a0_mr_a0;
extern const struct iwl_cfg iwl_cfg_so_a0_hr_a0;
extern const struct iwl_cfg iwl_cfg_quz_a0_hr_b0;
extern const struct iwl_cfg iwl_cfg_bz_a0_hr_b0;
extern const struct iwl_cfg iwl_cfg_bz_a0_gf_a0;
extern const struct iwl_cfg iwl_cfg_bz_a0_gf4_a0;
extern const struct iwl_cfg iwl_cfg_bz_a0_mr_a0;
#endif /* CONFIG_IWLMVM */

#endif /* __IWL_CONFIG_H__ */
struct iwl_dev_info {
	u16 device;
	u16 subdevice;
	u16 mac_type;
	u16 rf_type;
	u8 mac_step;
	u8 rf_id;
	u8 no_160;
	u8 cores;
	u8 cdb;
	const struct iwl_cfg *cfg;
	const char *name;
};

/*
 * This list declares the config structures for all devices.
 */
extern const struct iwl_cfg_trans_params iwl9000_trans_cfg;
extern const struct iwl_cfg_trans_params iwl9560_trans_cfg;
extern const struct iwl_cfg_trans_params iwl9560_long_latency_trans_cfg;
extern const struct iwl_cfg_trans_params iwl9560_shared_clk_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_qnj_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_qu_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_qu_medium_latency_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_qu_long_latency_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_ax200_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_snj_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_so_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_so_long_latency_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_ma_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_bz_trans_cfg;
extern const char iwl9162_name[];
extern const char iwl9260_name[];
extern const char iwl9260_1_name[];
extern const char iwl9270_name[];
extern const char iwl9461_name[];
extern const char iwl9462_name[];
extern const char iwl9560_name[];
extern const char iwl9162_160_name[];
extern const char iwl9260_160_name[];
extern const char iwl9270_160_name[];
extern const char iwl9461_160_name[];
extern const char iwl9462_160_name[];
extern const char iwl9560_160_name[];
extern const char iwl9260_killer_1550_name[];
extern const char iwl9560_killer_1550i_name[];
extern const char iwl9560_killer_1550s_name[];
extern const char iwl_ax200_name[];
extern const char iwl_ax203_name[];
extern const char iwl_ax201_name[];
extern const char iwl_ax101_name[];
extern const char iwl_ax200_killer_1650w_name[];
extern const char iwl_ax200_killer_1650x_name[];
extern const char iwl_ax201_killer_1650s_name[];
extern const char iwl_ax201_killer_1650i_name[];
extern const char iwl_ax210_killer_1675w_name[];
extern const char iwl_ax210_killer_1675x_name[];
extern const char iwl9560_killer_1550i_160_name[];
extern const char iwl9560_killer_1550s_160_name[];
extern const char iwl_ax211_name[];
extern const char iwl_ax221_name[];
extern const char iwl_ax231_name[];
extern const char iwl_ax411_name[];
extern const char iwl_bz_name[];
#if IS_ENABLED(CONFIG_IWLDVM)
extern const struct iwl_cfg iwl5300_agn_cfg;
extern const struct iwl_cfg iwl5100_agn_cfg;
extern const struct iwl_cfg iwl5350_agn_cfg;
extern const struct iwl_cfg iwl5100_bgn_cfg;
extern const struct iwl_cfg iwl5100_abg_cfg;
extern const struct iwl_cfg iwl5150_agn_cfg;
extern const struct iwl_cfg iwl5150_abg_cfg;
extern const struct iwl_cfg iwl6005_2agn_cfg;
extern const struct iwl_cfg iwl6005_2abg_cfg;
extern const struct iwl_cfg iwl6005_2bg_cfg;
extern const struct iwl_cfg iwl6005_2agn_sff_cfg;
extern const struct iwl_cfg iwl6005_2agn_d_cfg;
extern const struct iwl_cfg iwl6005_2agn_mow1_cfg;
extern const struct iwl_cfg iwl6005_2agn_mow2_cfg;
extern const struct iwl_cfg iwl1030_bgn_cfg;
extern const struct iwl_cfg iwl1030_bg_cfg;
extern const struct iwl_cfg iwl6030_2agn_cfg;
extern const struct iwl_cfg iwl6030_2abg_cfg;
extern const struct iwl_cfg iwl6030_2bgn_cfg;
extern const struct iwl_cfg iwl6030_2bg_cfg;
extern const struct iwl_cfg iwl6000i_2agn_cfg;
extern const struct iwl_cfg iwl6000i_2abg_cfg;
extern const struct iwl_cfg iwl6000i_2bg_cfg;
extern const struct iwl_cfg iwl6000_3agn_cfg;
extern const struct iwl_cfg iwl6050_2agn_cfg;
extern const struct iwl_cfg iwl6050_2abg_cfg;
extern const struct iwl_cfg iwl6150_bgn_cfg;
extern const struct iwl_cfg iwl6150_bg_cfg;
extern const struct iwl_cfg iwl1000_bgn_cfg;
extern const struct iwl_cfg iwl1000_bg_cfg;
extern const struct iwl_cfg iwl100_bgn_cfg;
extern const struct iwl_cfg iwl100_bg_cfg;
extern const struct iwl_cfg iwl130_bgn_cfg;
extern const struct iwl_cfg iwl130_bg_cfg;
extern const struct iwl_cfg iwl2000_2bgn_cfg;
extern const struct iwl_cfg iwl2000_2bgn_d_cfg;
extern const struct iwl_cfg iwl2030_2bgn_cfg;
extern const struct iwl_cfg iwl6035_2agn_cfg;
extern const struct iwl_cfg iwl6035_2agn_sff_cfg;
extern const struct iwl_cfg iwl105_bgn_cfg;
extern const struct iwl_cfg iwl105_bgn_d_cfg;
extern const struct iwl_cfg iwl135_bgn_cfg;
#endif /* CONFIG_IWLDVM */
#if IS_ENABLED(CONFIG_IWLMVM)
extern const struct iwl_cfg iwl7260_2ac_cfg;
extern const struct iwl_cfg iwl7260_2ac_cfg_high_temp;
extern const struct iwl_cfg iwl7260_2n_cfg;
extern const struct iwl_cfg iwl7260_n_cfg;
extern const struct iwl_cfg iwl3160_2ac_cfg;
extern const struct iwl_cfg iwl3160_2n_cfg;
extern const struct iwl_cfg iwl3160_n_cfg;
extern const struct iwl_cfg iwl3165_2ac_cfg;
extern const struct iwl_cfg iwl3168_2ac_cfg;
extern const struct iwl_cfg iwl7265_2ac_cfg;
extern const struct iwl_cfg iwl7265_2n_cfg;
extern const struct iwl_cfg iwl7265_n_cfg;
extern const struct iwl_cfg iwl7265d_2ac_cfg;
extern const struct iwl_cfg iwl7265d_2n_cfg;
extern const struct iwl_cfg iwl7265d_n_cfg;
extern const struct iwl_cfg iwl8260_2n_cfg;
extern const struct iwl_cfg iwl8260_2ac_cfg;
extern const struct iwl_cfg iwl8265_2ac_cfg;
extern const struct iwl_cfg iwl8275_2ac_cfg;
extern const struct iwl_cfg iwl4165_2ac_cfg;
extern const struct iwl_cfg iwl9260_2ac_cfg;
extern const struct iwl_cfg iwl9560_qu_b0_jf_b0_cfg;
extern const struct iwl_cfg iwl9560_qu_c0_jf_b0_cfg;
extern const struct iwl_cfg iwl9560_quz_a0_jf_b0_cfg;
extern const struct iwl_cfg iwl9560_qnj_b0_jf_b0_cfg;
extern const struct iwl_cfg iwl9560_2ac_cfg_soc;
extern const struct iwl_cfg iwl_qu_b0_hr1_b0;
extern const struct iwl_cfg iwl_qu_c0_hr1_b0;
extern const struct iwl_cfg iwl_quz_a0_hr1_b0;
extern const struct iwl_cfg iwl_qu_b0_hr_b0;
extern const struct iwl_cfg iwl_qu_c0_hr_b0;
extern const struct iwl_cfg iwl_ax200_cfg_cc;
extern const struct iwl_cfg iwl_ax201_cfg_qu_hr;
extern const struct iwl_cfg iwl_ax201_cfg_qu_c0_hr_b0;
extern const struct iwl_cfg iwl_ax201_cfg_quz_hr;
extern const struct iwl_cfg iwl_ax1650i_cfg_quz_hr;
extern const struct iwl_cfg iwl_ax1650s_cfg_quz_hr;
extern const struct iwl_cfg killer1650s_2ax_cfg_qu_b0_hr_b0;
extern const struct iwl_cfg killer1650i_2ax_cfg_qu_b0_hr_b0;
extern const struct iwl_cfg killer1650s_2ax_cfg_qu_c0_hr_b0;
extern const struct iwl_cfg killer1650i_2ax_cfg_qu_c0_hr_b0;
extern const struct iwl_cfg killer1650x_2ax_cfg;
extern const struct iwl_cfg killer1650w_2ax_cfg;
extern const struct iwl_cfg iwl_qnj_b0_hr_b0_cfg;
extern const struct iwl_cfg iwlax210_2ax_cfg_so_jf_b0;
extern const struct iwl_cfg iwlax210_2ax_cfg_so_hr_a0;
extern const struct iwl_cfg iwlax211_2ax_cfg_so_gf_a0;
extern const struct iwl_cfg iwlax211_2ax_cfg_so_gf_a0_long;
extern const struct iwl_cfg iwlax210_2ax_cfg_ty_gf_a0;
extern const struct iwl_cfg iwlax411_2ax_cfg_so_gf4_a0;
extern const struct iwl_cfg iwlax411_2ax_cfg_so_gf4_a0_long;
extern const struct iwl_cfg iwlax411_2ax_cfg_sosnj_gf4_a0;
extern const struct iwl_cfg iwlax211_cfg_snj_gf_a0;
extern const struct iwl_cfg iwl_cfg_snj_hr_b0;
extern const struct iwl_cfg iwl_cfg_snj_a0_jf_b0;
extern const struct iwl_cfg iwl_cfg_ma_a0_hr_b0;
extern const struct iwl_cfg iwl_cfg_ma_a0_gf_a0;
extern const struct iwl_cfg iwl_cfg_ma_a0_gf4_a0;
extern const struct iwl_cfg iwl_cfg_ma_a0_mr_a0;
extern const struct iwl_cfg iwl_cfg_ma_a0_fm_a0;
extern const struct iwl_cfg iwl_cfg_snj_a0_mr_a0;
extern const struct iwl_cfg iwl_cfg_so_a0_hr_a0;
extern const struct iwl_cfg iwl_cfg_quz_a0_hr_b0;
extern const struct iwl_cfg iwl_cfg_bz_a0_hr_b0;
extern const struct iwl_cfg iwl_cfg_bz_a0_gf_a0;
extern const struct iwl_cfg iwl_cfg_bz_a0_gf4_a0;
extern const struct iwl_cfg iwl_cfg_bz_a0_mr_a0;
#endif /* CONFIG_IWLMVM */

#endif /* __IWL_CONFIG_H__ */
struct iwl_dev_info {
	u16 device;
	u16 subdevice;
	u16 mac_type;
	u16 rf_type;
	u8 mac_step;
	u8 rf_id;
	u8 no_160;
	u8 cores;
	u8 cdb;
	const struct iwl_cfg *cfg;
	const char *name;
};

/*
 * This list declares the config structures for all devices.
 */
extern const struct iwl_cfg_trans_params iwl9000_trans_cfg;
extern const struct iwl_cfg_trans_params iwl9560_trans_cfg;
extern const struct iwl_cfg_trans_params iwl9560_long_latency_trans_cfg;
extern const struct iwl_cfg_trans_params iwl9560_shared_clk_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_qnj_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_qu_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_qu_medium_latency_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_qu_long_latency_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_ax200_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_snj_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_so_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_so_long_latency_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_ma_trans_cfg;
extern const struct iwl_cfg_trans_params iwl_bz_trans_cfg;
extern const char iwl9162_name[];
extern const char iwl9260_name[];
extern const char iwl9260_1_name[];
extern const char iwl9270_name[];
extern const char iwl9461_name[];
extern const char iwl9462_name[];
extern const char iwl9560_name[];
extern const char iwl9162_160_name[];
extern const char iwl9260_160_name[];
extern const char iwl9270_160_name[];
extern const char iwl9461_160_name[];
extern const char iwl9462_160_name[];
extern const char iwl9560_160_name[];
extern const char iwl9260_killer_1550_name[];
extern const char iwl9560_killer_1550i_name[];
extern const char iwl9560_killer_1550s_name[];
extern const char iwl_ax200_name[];
extern const char iwl_ax203_name[];
extern const char iwl_ax201_name[];
extern const char iwl_ax101_name[];
extern const char iwl_ax200_killer_1650w_name[];
extern const char iwl_ax200_killer_1650x_name[];
extern const char iwl_ax201_killer_1650s_name[];
extern const char iwl_ax201_killer_1650i_name[];
extern const char iwl_ax210_killer_1675w_name[];
extern const char iwl_ax210_killer_1675x_name[];
extern const char iwl9560_killer_1550i_160_name[];
extern const char iwl9560_killer_1550s_160_name[];
extern const char iwl_ax211_name[];
extern const char iwl_ax221_name[];
extern const char iwl_ax231_name[];
extern const char iwl_ax411_name[];
extern const char iwl_bz_name[];
#if IS_ENABLED(CONFIG_IWLDVM)
extern const struct iwl_cfg iwl5300_agn_cfg;
extern const struct iwl_cfg iwl5100_agn_cfg;
extern const struct iwl_cfg iwl5350_agn_cfg;
extern const struct iwl_cfg iwl5100_bgn_cfg;
extern const struct iwl_cfg iwl5100_abg_cfg;
extern const struct iwl_cfg iwl5150_agn_cfg;
extern const struct iwl_cfg iwl5150_abg_cfg;
extern const struct iwl_cfg iwl6005_2agn_cfg;
extern const struct iwl_cfg iwl6005_2abg_cfg;
extern const struct iwl_cfg iwl6005_2bg_cfg;
extern const struct iwl_cfg iwl6005_2agn_sff_cfg;
extern const struct iwl_cfg iwl6005_2agn_d_cfg;
extern const struct iwl_cfg iwl6005_2agn_mow1_cfg;
extern const struct iwl_cfg iwl6005_2agn_mow2_cfg;
extern const struct iwl_cfg iwl1030_bgn_cfg;
extern const struct iwl_cfg iwl1030_bg_cfg;
extern const struct iwl_cfg iwl6030_2agn_cfg;
extern const struct iwl_cfg iwl6030_2abg_cfg;
extern const struct iwl_cfg iwl6030_2bgn_cfg;
extern const struct iwl_cfg iwl6030_2bg_cfg;
extern const struct iwl_cfg iwl6000i_2agn_cfg;
extern const struct iwl_cfg iwl6000i_2abg_cfg;
extern const struct iwl_cfg iwl6000i_2bg_cfg;
extern const struct iwl_cfg iwl6000_3agn_cfg;
extern const struct iwl_cfg iwl6050_2agn_cfg;
extern const struct iwl_cfg iwl6050_2abg_cfg;
extern const struct iwl_cfg iwl6150_bgn_cfg;
extern const struct iwl_cfg iwl6150_bg_cfg;
extern const struct iwl_cfg iwl1000_bgn_cfg;
extern const struct iwl_cfg iwl1000_bg_cfg;
extern const struct iwl_cfg iwl100_bgn_cfg;
extern const struct iwl_cfg iwl100_bg_cfg;
extern const struct iwl_cfg iwl130_bgn_cfg;
extern const struct iwl_cfg iwl130_bg_cfg;
extern const struct iwl_cfg iwl2000_2bgn_cfg;
extern const struct iwl_cfg iwl2000_2bgn_d_cfg;
extern const struct iwl_cfg iwl2030_2bgn_cfg;
extern const struct iwl_cfg iwl6035_2agn_cfg;
extern const struct iwl_cfg iwl6035_2agn_sff_cfg;
extern const struct iwl_cfg iwl105_bgn_cfg;
extern const struct iwl_cfg iwl105_bgn_d_cfg;
extern const struct iwl_cfg iwl135_bgn_cfg;
#endif /* CONFIG_IWLDVM */
#if IS_ENABLED(CONFIG_IWLMVM)
extern const struct iwl_cfg iwl7260_2ac_cfg;
extern const struct iwl_cfg iwl7260_2ac_cfg_high_temp;
extern const struct iwl_cfg iwl7260_2n_cfg;
extern const struct iwl_cfg iwl7260_n_cfg;
extern const struct iwl_cfg iwl3160_2ac_cfg;
extern const struct iwl_cfg iwl3160_2n_cfg;
extern const struct iwl_cfg iwl3160_n_cfg;
extern const struct iwl_cfg iwl3165_2ac_cfg;
extern const struct iwl_cfg iwl3168_2ac_cfg;
extern const struct iwl_cfg iwl7265_2ac_cfg;
extern const struct iwl_cfg iwl7265_2n_cfg;
extern const struct iwl_cfg iwl7265_n_cfg;
extern const struct iwl_cfg iwl7265d_2ac_cfg;
extern const struct iwl_cfg iwl7265d_2n_cfg;
extern const struct iwl_cfg iwl7265d_n_cfg;
extern const struct iwl_cfg iwl8260_2n_cfg;
extern const struct iwl_cfg iwl8260_2ac_cfg;
extern const struct iwl_cfg iwl8265_2ac_cfg;
extern const struct iwl_cfg iwl8275_2ac_cfg;
extern const struct iwl_cfg iwl4165_2ac_cfg;
extern const struct iwl_cfg iwl9260_2ac_cfg;
extern const struct iwl_cfg iwl9560_qu_b0_jf_b0_cfg;
extern const struct iwl_cfg iwl9560_qu_c0_jf_b0_cfg;
extern const struct iwl_cfg iwl9560_quz_a0_jf_b0_cfg;
extern const struct iwl_cfg iwl9560_qnj_b0_jf_b0_cfg;
extern const struct iwl_cfg iwl9560_2ac_cfg_soc;
extern const struct iwl_cfg iwl_qu_b0_hr1_b0;
extern const struct iwl_cfg iwl_qu_c0_hr1_b0;
extern const struct iwl_cfg iwl_quz_a0_hr1_b0;
extern const struct iwl_cfg iwl_qu_b0_hr_b0;
extern const struct iwl_cfg iwl_qu_c0_hr_b0;
extern const struct iwl_cfg iwl_ax200_cfg_cc;
extern const struct iwl_cfg iwl_ax201_cfg_qu_hr;
extern const struct iwl_cfg iwl_ax201_cfg_qu_c0_hr_b0;
extern const struct iwl_cfg iwl_ax201_cfg_quz_hr;
extern const struct iwl_cfg iwl_ax1650i_cfg_quz_hr;
extern const struct iwl_cfg iwl_ax1650s_cfg_quz_hr;
extern const struct iwl_cfg killer1650s_2ax_cfg_qu_b0_hr_b0;
extern const struct iwl_cfg killer1650i_2ax_cfg_qu_b0_hr_b0;
extern const struct iwl_cfg killer1650s_2ax_cfg_qu_c0_hr_b0;
extern const struct iwl_cfg killer1650i_2ax_cfg_qu_c0_hr_b0;
extern const struct iwl_cfg killer1650x_2ax_cfg;
extern const struct iwl_cfg killer1650w_2ax_cfg;
extern const struct iwl_cfg iwl_qnj_b0_hr_b0_cfg;
extern const struct iwl_cfg iwlax210_2ax_cfg_so_jf_b0;
extern const struct iwl_cfg iwlax210_2ax_cfg_so_hr_a0;
extern const struct iwl_cfg iwlax211_2ax_cfg_so_gf_a0;
extern const struct iwl_cfg iwlax211_2ax_cfg_so_gf_a0_long;
extern const struct iwl_cfg iwlax210_2ax_cfg_ty_gf_a0;
extern const struct iwl_cfg iwlax411_2ax_cfg_so_gf4_a0;
extern const struct iwl_cfg iwlax411_2ax_cfg_so_gf4_a0_long;
extern const struct iwl_cfg iwlax411_2ax_cfg_sosnj_gf4_a0;
extern const struct iwl_cfg iwlax211_cfg_snj_gf_a0;
extern const struct iwl_cfg iwl_cfg_snj_hr_b0;
extern const struct iwl_cfg iwl_cfg_snj_a0_jf_b0;
extern const struct iwl_cfg iwl_cfg_ma_a0_hr_b0;
extern const struct iwl_cfg iwl_cfg_ma_a0_gf_a0;
extern const struct iwl_cfg iwl_cfg_ma_a0_gf4_a0;
extern const struct iwl_cfg iwl_cfg_ma_a0_mr_a0;
extern const struct iwl_cfg iwl_cfg_ma_a0_fm_a0;
extern const struct iwl_cfg iwl_cfg_snj_a0_mr_a0;
extern const struct iwl_cfg iwl_cfg_so_a0_hr_a0;
extern const struct iwl_cfg iwl_cfg_quz_a0_hr_b0;
extern const struct iwl_cfg iwl_cfg_bz_a0_hr_b0;
extern const struct iwl_cfg iwl_cfg_bz_a0_gf_a0;
extern const struct iwl_cfg iwl_cfg_bz_a0_gf4_a0;
extern const struct iwl_cfg iwl_cfg_bz_a0_mr_a0;
#endif /* CONFIG_IWLMVM */

#endif /* __IWL_CONFIG_H__ */