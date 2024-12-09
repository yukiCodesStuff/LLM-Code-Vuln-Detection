	return 0;
}

static int iwl_pcie_gen2_force_power_gating(struct iwl_trans *trans)
{
	int ret;

	ret = iwl_finish_nic_init(trans, trans->trans_cfg);
	if (ret < 0)
		return ret;

	iwl_set_bits_prph(trans, HPM_HIPM_GEN_CFG,
			  HPM_HIPM_GEN_CFG_CR_FORCE_ACTIVE);
	udelay(20);
	iwl_set_bits_prph(trans, HPM_HIPM_GEN_CFG,
			  HPM_HIPM_GEN_CFG_CR_PG_EN |
			  HPM_HIPM_GEN_CFG_CR_SLP_EN);
	udelay(20);
	iwl_clear_bits_prph(trans, HPM_HIPM_GEN_CFG,
			    HPM_HIPM_GEN_CFG_CR_FORCE_ACTIVE);

	iwl_trans_pcie_sw_reset(trans);

	return 0;
}

static int _iwl_trans_pcie_start_hw(struct iwl_trans *trans)
{
	struct iwl_trans_pcie *trans_pcie = IWL_TRANS_GET_PCIE_TRANS(trans);
	int err;

	iwl_trans_pcie_sw_reset(trans);

	if (trans->trans_cfg->device_family == IWL_DEVICE_FAMILY_22000 &&
	    trans->cfg->integrated) {
		err = iwl_pcie_gen2_force_power_gating(trans);
		if (err)
			return err;
	}

	err = iwl_pcie_apm_init(trans);
	if (err)
		return err;
