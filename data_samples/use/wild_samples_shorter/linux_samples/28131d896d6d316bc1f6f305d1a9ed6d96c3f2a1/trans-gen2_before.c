
	iwl_pcie_apm_config(trans);

	ret = iwl_finish_nic_init(trans, trans->trans_cfg);
	if (ret)
		return ret;

	set_bit(STATUS_DEVICE_ENABLED, &trans->status);
	if (trans_pcie->is_down)
		return;

	if (trans->state >= IWL_TRANS_FW_STARTED) {
		if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_BZ) {
			iwl_set_bit(trans, CSR_GP_CNTRL,
				    CSR_GP_CNTRL_REG_FLAG_BUS_MASTER_DISABLE_REQ);
			iwl_poll_bit(trans, CSR_GP_CNTRL,
				     CSR_GP_CNTRL_REG_FLAG_BUS_MASTER_DISABLE_STATUS,
				     CSR_GP_CNTRL_REG_FLAG_BUS_MASTER_DISABLE_STATUS,
				     5000);
			msleep(100);
			iwl_set_bit(trans, CSR_GP_CNTRL,
				    CSR_GP_CNTRL_REG_FLAG_SW_RESET);
		} else if (trans_pcie->fw_reset_handshake) {
			iwl_trans_pcie_fw_reset_handshake(trans);
		}
	}

	trans_pcie->is_down = true;

	/* tell the device to stop sending interrupts */
	else
		iwl_pcie_ctxt_info_free(trans);

	/* Make sure (redundant) we've released our request to stay awake */
	if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_BZ)
		iwl_clear_bit(trans, CSR_GP_CNTRL,
			      CSR_GP_CNTRL_REG_FLAG_BZ_MAC_ACCESS_REQ);
	else
		iwl_clear_bit(trans, CSR_GP_CNTRL,
			      CSR_GP_CNTRL_REG_FLAG_MAC_ACCESS_REQ);

	if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_BZ) {
		iwl_set_bit(trans, CSR_GP_CNTRL,
			    CSR_GP_CNTRL_REG_FLAG_SW_RESET);
	}
	/* Stop the device, and put it in low power state */
	iwl_pcie_gen2_apm_stop(trans, false);

	iwl_trans_sw_reset(trans);

	iwl_pcie_set_ltr(trans);

	if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_BZ)
		iwl_set_bit(trans, CSR_GP_CNTRL,
			    CSR_GP_CNTRL_REG_FLAG_ROM_START);
	else if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_AX210)
		iwl_write_umac_prph(trans, UREG_CPU_INIT_RUN, 1);
	else
		iwl_write_prph(trans, UREG_CPU_INIT_RUN, 1);

	/* re-check RF-Kill state since we may have missed the interrupt */
	hw_rfkill = iwl_pcie_check_hw_rf_kill(trans);
	if (hw_rfkill && !run_in_rfkill)