/* Ma devices */
	{IWL_PCI_DEVICE(0x2729, PCI_ANY_ID, iwl_ma_trans_cfg)},
	{IWL_PCI_DEVICE(0x7E40, PCI_ANY_ID, iwl_ma_trans_cfg)},

/* Bz devices */
	{IWL_PCI_DEVICE(0x2727, PCI_ANY_ID, iwl_bz_trans_cfg)},
#endif /* CONFIG_IWLMVM */
#define IWL_DEV_INFO(_device, _subdevice, _cfg, _name) \
	_IWL_DEV_INFO(_device, _subdevice, IWL_CFG_ANY, IWL_CFG_ANY,	   \
		      IWL_CFG_ANY, IWL_CFG_ANY, IWL_CFG_ANY, IWL_CFG_ANY,  \
		      IWL_CFG_NO_CDB, _cfg, _name)

static const struct iwl_dev_info iwl_dev_info_table[] = {
#if IS_ENABLED(CONFIG_IWLMVM)
/* 9000 */
	IWL_DEV_INFO(0x31DC, 0x1552, iwl9560_2ac_cfg_soc, iwl9560_killer_1550i_name),
	IWL_DEV_INFO(0xA370, 0x1551, iwl9560_2ac_cfg_soc, iwl9560_killer_1550s_name),
	IWL_DEV_INFO(0xA370, 0x1552, iwl9560_2ac_cfg_soc, iwl9560_killer_1550i_name),
	IWL_DEV_INFO(0x51F0, 0x1552, iwl9560_2ac_cfg_soc, iwl9560_killer_1550s_160_name),
	IWL_DEV_INFO(0x51F0, 0x1551, iwl9560_2ac_cfg_soc, iwl9560_killer_1550i_160_name),

	IWL_DEV_INFO(0x271C, 0x0214, iwl9260_2ac_cfg, iwl9260_1_name),

/* AX200 */
	IWL_DEV_INFO(0x2723, 0x1653, iwl_ax200_cfg_cc, iwl_ax200_killer_1650w_name),
	IWL_DEV_INFO(0x2723, 0x1654, iwl_ax200_cfg_cc, iwl_ax200_killer_1650x_name),
	IWL_DEV_INFO(0x2723, IWL_CFG_ANY, iwl_ax200_cfg_cc, iwl_ax200_name),

	/* Qu with Hr */
	IWL_DEV_INFO(0x43F0, 0x0070, iwl_ax201_cfg_qu_hr, NULL),
	IWL_DEV_INFO(0x43F0, 0x0074, iwl_ax201_cfg_qu_hr, NULL),
	IWL_DEV_INFO(0x7AF0, 0x0510, iwlax211_2ax_cfg_so_gf_a0, NULL),
	IWL_DEV_INFO(0x7AF0, 0x0A10, iwlax211_2ax_cfg_so_gf_a0, NULL),

	/* SnJ with HR */
	IWL_DEV_INFO(0x2725, 0x00B0, iwlax411_2ax_cfg_sosnj_gf4_a0, NULL),
	IWL_DEV_INFO(0x2726, 0x0090, iwlax211_cfg_snj_gf_a0, NULL),
	IWL_DEV_INFO(0x2726, 0x0098, iwlax211_cfg_snj_gf_a0, NULL),
	IWL_DEV_INFO(0x2726, 0x0510, iwlax211_cfg_snj_gf_a0, NULL),
	IWL_DEV_INFO(0x2726, 0x1651, iwl_cfg_snj_hr_b0, iwl_ax201_killer_1650s_name),
	IWL_DEV_INFO(0x2726, 0x1652, iwl_cfg_snj_hr_b0, iwl_ax201_killer_1650i_name),

	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_PU, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_JF1, IWL_CFG_RF_ID_JF1,
		      IWL_CFG_NO_160, IWL_CFG_CORES_BT, IWL_CFG_NO_CDB,
		      iwl9260_2ac_cfg, iwl9462_name),

	_IWL_DEV_INFO(0x2526, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_PNJ, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_JF2, IWL_CFG_RF_ID_JF,
		      IWL_CFG_160, IWL_CFG_CORES_BT, IWL_CFG_NO_CDB,
		      iwl9260_2ac_cfg, iwl9560_160_name),
	_IWL_DEV_INFO(0x2526, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_PNJ, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_JF2, IWL_CFG_RF_ID_JF,
		      IWL_CFG_NO_160, IWL_CFG_CORES_BT, IWL_CFG_NO_CDB,
		      iwl9260_2ac_cfg, iwl9560_name),

	_IWL_DEV_INFO(0x2526, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_TH, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_TH, IWL_CFG_ANY,
		      IWL_CFG_160, IWL_CFG_CORES_BT_GNSS, IWL_CFG_NO_CDB,
		      IWL_CFG_RF_TYPE_HR2, IWL_CFG_ANY,
		      IWL_CFG_NO_160, IWL_CFG_ANY, IWL_CFG_NO_CDB,
		      iwl_cfg_so_a0_hr_a0, iwl_ax203_name),
	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_SO, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_HR2, IWL_CFG_ANY,
		      IWL_CFG_NO_160, IWL_CFG_ANY, IWL_CFG_NO_CDB,
		      iwl_cfg_so_a0_hr_a0, iwl_ax203_name),
	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_SO, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_HR1, IWL_CFG_ANY,
		      IWL_CFG_160, IWL_CFG_ANY, IWL_CFG_NO_CDB,
		      IWL_CFG_RF_TYPE_MR, IWL_CFG_ANY,
		      IWL_CFG_ANY, IWL_CFG_ANY, IWL_CFG_NO_CDB,
		      iwl_cfg_bz_a0_mr_a0, iwl_bz_name),

/* SoF with JF2 */
	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_SOF, IWL_CFG_ANY,
#endif /* CONFIG_IWLMVM */
};

/* PCI registers */
#define PCI_CFG_RETRY_TIMEOUT	0x041

static int iwl_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	const struct iwl_cfg_trans_params *trans;
	const struct iwl_cfg *cfg_7265d __maybe_unused = NULL;
	struct iwl_trans *iwl_trans;
	struct iwl_trans_pcie *trans_pcie;
	int i, ret;
	const struct iwl_cfg *cfg;

	trans = (void *)(ent->driver_data & ~TRANS_CFG_MARKER);


	trans_pcie = IWL_TRANS_GET_PCIE_TRANS(iwl_trans);

	iwl_trans->hw_rf_id = iwl_read32(iwl_trans, CSR_HW_RF_ID);

	for (i = 0; i < ARRAY_SIZE(iwl_dev_info_table); i++) {
		const struct iwl_dev_info *dev_info = &iwl_dev_info_table[i];
		if ((dev_info->device == (u16)IWL_CFG_ANY ||
		     dev_info->device == pdev->device) &&
		    (dev_info->subdevice == (u16)IWL_CFG_ANY ||
		     dev_info->subdevice == pdev->subsystem_device) &&
		    (dev_info->mac_type == (u16)IWL_CFG_ANY ||
		     dev_info->mac_type ==
		     CSR_HW_REV_TYPE(iwl_trans->hw_rev)) &&
		    (dev_info->mac_step == (u8)IWL_CFG_ANY ||
		     dev_info->mac_step ==
		     CSR_HW_REV_STEP(iwl_trans->hw_rev)) &&
		    (dev_info->rf_type == (u16)IWL_CFG_ANY ||
		     dev_info->rf_type ==
		     CSR_HW_RFID_TYPE(iwl_trans->hw_rf_id)) &&
		    (dev_info->cdb == IWL_CFG_NO_CDB ||
		     CSR_HW_RFID_IS_CDB(iwl_trans->hw_rf_id)) &&
		    (dev_info->rf_id == (u8)IWL_CFG_ANY ||
		     dev_info->rf_id ==
		     IWL_SUBDEVICE_RF_ID(pdev->subsystem_device)) &&
		    (dev_info->no_160 == (u8)IWL_CFG_ANY ||
		     dev_info->no_160 ==
		     IWL_SUBDEVICE_NO_160(pdev->subsystem_device)) &&
		    (dev_info->cores == (u8)IWL_CFG_ANY ||
		     dev_info->cores ==
		     IWL_SUBDEVICE_CORES(pdev->subsystem_device))) {
			iwl_trans->cfg = dev_info->cfg;
			iwl_trans->name = dev_info->name;
		}
	}

#if IS_ENABLED(CONFIG_IWLMVM)
