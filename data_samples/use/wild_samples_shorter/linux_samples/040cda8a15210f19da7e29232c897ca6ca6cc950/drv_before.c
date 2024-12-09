
	/* same thing for QuZ... */
	if (iwl_trans->hw_rev == CSR_HW_REV_TYPE_QUZ) {
		if (iwl_trans->cfg == &iwl_ax101_cfg_qu_hr)
			iwl_trans->cfg = &iwl_ax101_cfg_quz_hr;
		else if (iwl_trans->cfg == &iwl_ax201_cfg_qu_hr)
			iwl_trans->cfg = &iwl_ax201_cfg_quz_hr;
		else if (iwl_trans->cfg == &iwl9461_2ac_cfg_qu_b0_jf_b0)
			iwl_trans->cfg = &iwl9461_2ac_cfg_quz_a0_jf_b0_soc;
		else if (iwl_trans->cfg == &iwl9462_2ac_cfg_qu_b0_jf_b0)
			iwl_trans->cfg = &iwl9462_2ac_cfg_quz_a0_jf_b0_soc;
		else if (iwl_trans->cfg == &iwl9560_2ac_cfg_qu_b0_jf_b0)
			iwl_trans->cfg = &iwl9560_2ac_cfg_quz_a0_jf_b0_soc;
		else if (iwl_trans->cfg == &iwl9560_2ac_160_cfg_qu_b0_jf_b0)
			iwl_trans->cfg = &iwl9560_2ac_160_cfg_quz_a0_jf_b0_soc;
	}

#endif
	/* now set the real cfg we decided to use */