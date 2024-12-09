/* Ma devices */
	{IWL_PCI_DEVICE(0x2729, PCI_ANY_ID, iwl_ma_trans_cfg)},
	{IWL_PCI_DEVICE(0x7E40, PCI_ANY_ID, iwl_ma_trans_cfg)},
	{IWL_PCI_DEVICE(0x7F70, PCI_ANY_ID, iwl_ma_trans_cfg)},

/* Bz devices */
	{IWL_PCI_DEVICE(0x2727, PCI_ANY_ID, iwl_bz_trans_cfg)},
#endif /* CONFIG_IWLMVM */
#define IWL_DEV_INFO(_device, _subdevice, _cfg, _name) \
	_IWL_DEV_INFO(_device, _subdevice, IWL_CFG_ANY, IWL_CFG_ANY,	   \
		      IWL_CFG_ANY, IWL_CFG_ANY, IWL_CFG_ANY, IWL_CFG_ANY,  \
		      IWL_CFG_ANY, _cfg, _name)

static const struct iwl_dev_info iwl_dev_info_table[] = {
#if IS_ENABLED(CONFIG_IWLMVM)
/* 9000 */
	IWL_DEV_INFO(0x31DC, 0x1552, iwl9560_2ac_cfg_soc, iwl9560_killer_1550i_name),
	IWL_DEV_INFO(0xA370, 0x1551, iwl9560_2ac_cfg_soc, iwl9560_killer_1550s_name),
	IWL_DEV_INFO(0xA370, 0x1552, iwl9560_2ac_cfg_soc, iwl9560_killer_1550i_name),
	IWL_DEV_INFO(0x54F0, 0x1551, iwl9560_2ac_cfg_soc, iwl9560_killer_1550s_160_name),
	IWL_DEV_INFO(0x54F0, 0x1552, iwl9560_2ac_cfg_soc, iwl9560_killer_1550i_name),
	IWL_DEV_INFO(0x51F0, 0x1552, iwl9560_2ac_cfg_soc, iwl9560_killer_1550s_160_name),
	IWL_DEV_INFO(0x51F0, 0x1551, iwl9560_2ac_cfg_soc, iwl9560_killer_1550i_160_name),
	IWL_DEV_INFO(0x51F0, 0x1691, iwlax411_2ax_cfg_so_gf4_a0, iwl_ax411_killer_1690s_name),
	IWL_DEV_INFO(0x51F0, 0x1692, iwlax411_2ax_cfg_so_gf4_a0, iwl_ax411_killer_1690i_name),
	IWL_DEV_INFO(0x54F0, 0x1691, iwlax411_2ax_cfg_so_gf4_a0, iwl_ax411_killer_1690s_name),
	IWL_DEV_INFO(0x54F0, 0x1692, iwlax411_2ax_cfg_so_gf4_a0, iwl_ax411_killer_1690i_name),
	IWL_DEV_INFO(0x7A70, 0x1691, iwlax411_2ax_cfg_so_gf4_a0, iwl_ax411_killer_1690s_name),
	IWL_DEV_INFO(0x7A70, 0x1692, iwlax411_2ax_cfg_so_gf4_a0, iwl_ax411_killer_1690i_name),

	IWL_DEV_INFO(0x271C, 0x0214, iwl9260_2ac_cfg, iwl9260_1_name),
	IWL_DEV_INFO(0x7E40, 0x1691, iwl_cfg_ma_a0_gf4_a0, iwl_ax411_killer_1690s_name),
	IWL_DEV_INFO(0x7E40, 0x1692, iwl_cfg_ma_a0_gf4_a0, iwl_ax411_killer_1690i_name),

/* AX200 */
	IWL_DEV_INFO(0x2723, IWL_CFG_ANY, iwl_ax200_cfg_cc, iwl_ax200_name),
	IWL_DEV_INFO(0x2723, 0x1653, iwl_ax200_cfg_cc, iwl_ax200_killer_1650w_name),
	IWL_DEV_INFO(0x2723, 0x1654, iwl_ax200_cfg_cc, iwl_ax200_killer_1650x_name),

	/* Qu with Hr */
	IWL_DEV_INFO(0x43F0, 0x0070, iwl_ax201_cfg_qu_hr, NULL),
	IWL_DEV_INFO(0x43F0, 0x0074, iwl_ax201_cfg_qu_hr, NULL),
	IWL_DEV_INFO(0x7AF0, 0x0510, iwlax211_2ax_cfg_so_gf_a0, NULL),
	IWL_DEV_INFO(0x7AF0, 0x0A10, iwlax211_2ax_cfg_so_gf_a0, NULL),

	/* So with JF */
	IWL_DEV_INFO(0x7A70, 0x1551, iwl9560_2ac_cfg_soc, iwl9560_killer_1550s_160_name),
	IWL_DEV_INFO(0x7A70, 0x1552, iwl9560_2ac_cfg_soc, iwl9560_killer_1550i_160_name),
	IWL_DEV_INFO(0x7AF0, 0x1551, iwl9560_2ac_cfg_soc, iwl9560_killer_1550s_160_name),
	IWL_DEV_INFO(0x7AF0, 0x1552, iwl9560_2ac_cfg_soc, iwl9560_killer_1550i_160_name),

	/* SnJ with HR */
	IWL_DEV_INFO(0x2725, 0x00B0, iwlax411_2ax_cfg_sosnj_gf4_a0, NULL),
	IWL_DEV_INFO(0x2726, 0x0090, iwlax211_cfg_snj_gf_a0, NULL),
	IWL_DEV_INFO(0x2726, 0x0098, iwlax211_cfg_snj_gf_a0, NULL),
	IWL_DEV_INFO(0x2726, 0x0510, iwlax211_cfg_snj_gf_a0, NULL),
	IWL_DEV_INFO(0x2726, 0x1651, iwl_cfg_snj_hr_b0, iwl_ax201_killer_1650s_name),
	IWL_DEV_INFO(0x2726, 0x1652, iwl_cfg_snj_hr_b0, iwl_ax201_killer_1650i_name),
	IWL_DEV_INFO(0x2726, 0x1671, iwlax211_cfg_snj_gf_a0, iwl_ax211_killer_1675s_name),
	IWL_DEV_INFO(0x2726, 0x1672, iwlax211_cfg_snj_gf_a0, iwl_ax211_killer_1675i_name),
	IWL_DEV_INFO(0x2726, 0x1691, iwlax411_2ax_cfg_sosnj_gf4_a0, iwl_ax411_killer_1690s_name),
	IWL_DEV_INFO(0x2726, 0x1692, iwlax411_2ax_cfg_sosnj_gf4_a0, iwl_ax411_killer_1690i_name),
	IWL_DEV_INFO(0x7F70, 0x1691, iwlax411_2ax_cfg_sosnj_gf4_a0, iwl_ax411_killer_1690s_name),
	IWL_DEV_INFO(0x7F70, 0x1692, iwlax411_2ax_cfg_sosnj_gf4_a0, iwl_ax411_killer_1690i_name),

	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_PU, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_JF1, IWL_CFG_RF_ID_JF1,
		      IWL_CFG_NO_160, IWL_CFG_CORES_BT, IWL_CFG_NO_CDB,
		      iwl9260_2ac_cfg, iwl9462_name),

	_IWL_DEV_INFO(0x2526, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_TH, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_TH, IWL_CFG_ANY,
		      IWL_CFG_160, IWL_CFG_CORES_BT_GNSS, IWL_CFG_NO_CDB,
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
	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_BZ, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_FM, IWL_CFG_ANY,
		      IWL_CFG_ANY, IWL_CFG_ANY, IWL_CFG_NO_CDB,
		      iwl_cfg_bz_a0_fm_a0, iwl_bz_name),
	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_GL, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_FM, IWL_CFG_ANY,
		      IWL_CFG_ANY, IWL_CFG_ANY, IWL_CFG_NO_CDB,
		      iwl_cfg_gl_a0_fm_a0, iwl_bz_name),

/* SoF with JF2 */
	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_SOF, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_JF2, IWL_CFG_RF_ID_JF,
		      IWL_CFG_160, IWL_CFG_CORES_BT, IWL_CFG_NO_CDB,
		      iwlax210_2ax_cfg_so_jf_b0, iwl9560_160_name),
	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_SOF, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_JF2, IWL_CFG_RF_ID_JF,
		      IWL_CFG_NO_160, IWL_CFG_CORES_BT, IWL_CFG_NO_CDB,
		      iwlax210_2ax_cfg_so_jf_b0, iwl9560_name),

/* SoF with JF */
	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_SOF, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_JF1, IWL_CFG_RF_ID_JF1,
		      IWL_CFG_160, IWL_CFG_CORES_BT, IWL_CFG_NO_CDB,
		      iwlax210_2ax_cfg_so_jf_b0, iwl9461_160_name),
	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_SOF, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_JF1, IWL_CFG_RF_ID_JF1_DIV,
		      IWL_CFG_160, IWL_CFG_CORES_BT, IWL_CFG_NO_CDB,
		      iwlax210_2ax_cfg_so_jf_b0, iwl9462_160_name),
	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_SOF, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_JF1, IWL_CFG_RF_ID_JF1,
		      IWL_CFG_NO_160, IWL_CFG_CORES_BT, IWL_CFG_NO_CDB,
		      iwlax210_2ax_cfg_so_jf_b0, iwl9461_name),
	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_SOF, IWL_CFG_ANY,
		      IWL_CFG_RF_TYPE_JF1, IWL_CFG_RF_ID_JF1_DIV,
		      IWL_CFG_NO_160, IWL_CFG_CORES_BT, IWL_CFG_NO_CDB,
		      iwlax210_2ax_cfg_so_jf_b0, iwl9462_name),

/* SoF with JF2 */
	_IWL_DEV_INFO(IWL_CFG_ANY, IWL_CFG_ANY,
		      IWL_CFG_MAC_TYPE_SOF, IWL_CFG_ANY,
#endif /* CONFIG_IWLMVM */
};

/*
 * In case that there is no OTP on the NIC, get the rf id and cdb info
 * from the prph registers.
 */
static int get_crf_id(struct iwl_trans *iwl_trans)
{
	int ret = 0;
	u32 wfpm_ctrl_addr;
	u32 wfpm_otp_cfg_addr;
	u32 sd_reg_ver_addr;
	u32 cdb = 0;
	u32 val;

	if (iwl_trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_AX210) {
		wfpm_ctrl_addr = WFPM_CTRL_REG_GEN2;
		wfpm_otp_cfg_addr = WFPM_OTP_CFG1_ADDR_GEN2;
		sd_reg_ver_addr = SD_REG_VER_GEN2;
	/* Qu/Pu families have other addresses */
	} else {
		wfpm_ctrl_addr = WFPM_CTRL_REG;
		wfpm_otp_cfg_addr = WFPM_OTP_CFG1_ADDR;
		sd_reg_ver_addr = SD_REG_VER;
	}

	if (!iwl_trans_grab_nic_access(iwl_trans)) {
		IWL_ERR(iwl_trans, "Failed to grab nic access before reading crf id\n");
		ret = -EIO;
		goto out;
	}

	/* Enable access to peripheral registers */
	val = iwl_read_umac_prph_no_grab(iwl_trans, wfpm_ctrl_addr);
	val |= ENABLE_WFPM;
	iwl_write_umac_prph_no_grab(iwl_trans, wfpm_ctrl_addr, val);

	/* Read crf info */
	val = iwl_read_prph_no_grab(iwl_trans, sd_reg_ver_addr);

	/* Read cdb info (also contains the jacket info if needed in the future */
	cdb = iwl_read_umac_prph_no_grab(iwl_trans, wfpm_otp_cfg_addr);

	/* Map between crf id to rf id */
	switch (REG_CRF_ID_TYPE(val)) {
	case REG_CRF_ID_TYPE_JF_1:
		iwl_trans->hw_rf_id = (IWL_CFG_RF_TYPE_JF1 << 12);
		break;
	case REG_CRF_ID_TYPE_JF_2:
		iwl_trans->hw_rf_id = (IWL_CFG_RF_TYPE_JF2 << 12);
		break;
	case REG_CRF_ID_TYPE_HR_NONE_CDB:
		iwl_trans->hw_rf_id = (IWL_CFG_RF_TYPE_HR1 << 12);
		break;
	case REG_CRF_ID_TYPE_HR_CDB:
		iwl_trans->hw_rf_id = (IWL_CFG_RF_TYPE_HR2 << 12);
		break;
	case REG_CRF_ID_TYPE_GF:
		iwl_trans->hw_rf_id = (IWL_CFG_RF_TYPE_GF << 12);
		break;
	case REG_CRF_ID_TYPE_MR:
		iwl_trans->hw_rf_id = (IWL_CFG_RF_TYPE_MR << 12);
		break;
		case REG_CRF_ID_TYPE_FM:
			iwl_trans->hw_rf_id = (IWL_CFG_RF_TYPE_FM << 12);
			break;
	default:
		ret = -EIO;
		IWL_ERR(iwl_trans,
			"Can find a correct rfid for crf id 0x%x\n",
			REG_CRF_ID_TYPE(val));
		goto out_release;

	}

	/* Set CDB capabilities */
	if (cdb & BIT(4)) {
		iwl_trans->hw_rf_id += BIT(28);
		IWL_INFO(iwl_trans, "Adding cdb to rf id\n");
	}

	IWL_INFO(iwl_trans, "Detected RF 0x%x from crf id 0x%x\n",
		 iwl_trans->hw_rf_id, REG_CRF_ID_TYPE(val));

out_release:
	iwl_trans_release_nic_access(iwl_trans);

out:
	return ret;
}

/* PCI registers */
#define PCI_CFG_RETRY_TIMEOUT	0x041

static const struct iwl_dev_info *
iwl_pci_find_dev_info(u16 device, u16 subsystem_device,
		      u16 mac_type, u8 mac_step,
		      u16 rf_type, u8 cdb, u8 rf_id, u8 no_160, u8 cores)
{
	int i;

	for (i = ARRAY_SIZE(iwl_dev_info_table) - 1; i >= 0; i--) {
		const struct iwl_dev_info *dev_info = &iwl_dev_info_table[i];

		if (dev_info->device != (u16)IWL_CFG_ANY &&
		    dev_info->device != device)
			continue;

		if (dev_info->subdevice != (u16)IWL_CFG_ANY &&
		    dev_info->subdevice != subsystem_device)
			continue;

		if (dev_info->mac_type != (u16)IWL_CFG_ANY &&
		    dev_info->mac_type != mac_type)
			continue;

		if (dev_info->mac_step != (u8)IWL_CFG_ANY &&
		    dev_info->mac_step != mac_step)
			continue;

		if (dev_info->rf_type != (u16)IWL_CFG_ANY &&
		    dev_info->rf_type != rf_type)
			continue;

		if (dev_info->cdb != (u8)IWL_CFG_ANY &&
		    dev_info->cdb != cdb)
			continue;

		if (dev_info->rf_id != (u8)IWL_CFG_ANY &&
		    dev_info->rf_id != rf_id)
			continue;

		if (dev_info->no_160 != (u8)IWL_CFG_ANY &&
		    dev_info->no_160 != no_160)
			continue;

		if (dev_info->cores != (u8)IWL_CFG_ANY &&
		    dev_info->cores != cores)
			continue;

		return dev_info;
	}

	return NULL;
}

static int iwl_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	const struct iwl_cfg_trans_params *trans;
	const struct iwl_cfg *cfg_7265d __maybe_unused = NULL;
	const struct iwl_dev_info *dev_info;
	struct iwl_trans *iwl_trans;
	struct iwl_trans_pcie *trans_pcie;
	int ret;
	const struct iwl_cfg *cfg;

	trans = (void *)(ent->driver_data & ~TRANS_CFG_MARKER);


	trans_pcie = IWL_TRANS_GET_PCIE_TRANS(iwl_trans);

	/*
	 * Let's try to grab NIC access early here. Sometimes, NICs may
	 * fail to initialize, and if that happens it's better if we see
	 * issues early on (and can reprobe, per the logic inside), than
	 * first trying to load the firmware etc. and potentially only
	 * detecting any problems when the first interface is brought up.
	 */
	ret = iwl_finish_nic_init(iwl_trans);
	if (ret)
		goto out_free_trans;
	if (iwl_trans_grab_nic_access(iwl_trans)) {
		/* all good */
		iwl_trans_release_nic_access(iwl_trans);
	} else {
		ret = -EIO;
		goto out_free_trans;
	}

	iwl_trans->hw_rf_id = iwl_read32(iwl_trans, CSR_HW_RF_ID);

	/*
	 * The RF_ID is set to zero in blank OTP so read version to
	 * extract the RF_ID.
	 * This is relevant only for family 9000 and up.
	 */
	if (iwl_trans->trans_cfg->rf_id &&
	    iwl_trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_9000 &&
	    !CSR_HW_RFID_TYPE(iwl_trans->hw_rf_id) && get_crf_id(iwl_trans))
		goto out_free_trans;

	dev_info = iwl_pci_find_dev_info(pdev->device, pdev->subsystem_device,
					 CSR_HW_REV_TYPE(iwl_trans->hw_rev),
					 CSR_HW_REV_STEP(iwl_trans->hw_rev),
					 CSR_HW_RFID_TYPE(iwl_trans->hw_rf_id),
					 CSR_HW_RFID_IS_CDB(iwl_trans->hw_rf_id),
					 IWL_SUBDEVICE_RF_ID(pdev->subsystem_device),
					 IWL_SUBDEVICE_NO_160(pdev->subsystem_device),
					 IWL_SUBDEVICE_CORES(pdev->subsystem_device));

	if (dev_info) {
		iwl_trans->cfg = dev_info->cfg;
		iwl_trans->name = dev_info->name;
	}

#if IS_ENABLED(CONFIG_IWLMVM)
