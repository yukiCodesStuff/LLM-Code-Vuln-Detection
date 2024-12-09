		if (wprot_val & PREG_WFPM_ACCESS) {
			IWL_ERR(trans,
				"Error, can not clear persistence bit\n");
			return -EPERM;
		}
		iwl_write_umac_prph_no_grab(trans, HPM_DEBUG,
					    hpm & ~PERSISTENCE_BIT);
	}

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

	lockdep_assert_held(&trans_pcie->mutex);

	err = iwl_pcie_prepare_card_hw(trans);
	if (err) {
		IWL_ERR(trans, "Error while preparing HW: %d\n", err);
		return err;
	}
	if (err) {
		IWL_ERR(trans, "Error while preparing HW: %d\n", err);
		return err;
	}

	err = iwl_trans_pcie_clear_persistence_bit(trans);
	if (err)
		return err;

	iwl_trans_pcie_sw_reset(trans);

	if (trans->trans_cfg->device_family == IWL_DEVICE_FAMILY_22000 &&
	    trans->cfg->integrated) {
		err = iwl_pcie_gen2_force_power_gating(trans);
		if (err)
			return err;
	}