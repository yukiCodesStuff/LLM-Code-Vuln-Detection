
	iwl_pcie_apm_config(trans);

	ret = iwl_finish_nic_init(trans);
	if (ret)
		return ret;

	set_bit(STATUS_DEVICE_ENABLED, &trans->status);
	if (trans_pcie->is_down)
		return;

	if (trans->state >= IWL_TRANS_FW_STARTED)
		if (trans_pcie->fw_reset_handshake)
			iwl_trans_pcie_fw_reset_handshake(trans);

	trans_pcie->is_down = true;

	/* tell the device to stop sending interrupts */
	else
		iwl_pcie_ctxt_info_free(trans);

	/* Stop the device, and put it in low power state */
	iwl_pcie_gen2_apm_stop(trans, false);

	iwl_trans_sw_reset(trans);

	iwl_pcie_set_ltr(trans);

	if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_BZ) {
		iwl_write32(trans, CSR_FUNC_SCRATCH, CSR_FUNC_SCRATCH_INIT_VALUE);
		iwl_set_bit(trans, CSR_GP_CNTRL,
			    CSR_GP_CNTRL_REG_FLAG_ROM_START);
	} else if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_AX210) {
		iwl_write_umac_prph(trans, UREG_CPU_INIT_RUN, 1);
	} else {
		iwl_write_prph(trans, UREG_CPU_INIT_RUN, 1);
	}

	/* re-check RF-Kill state since we may have missed the interrupt */
	hw_rfkill = iwl_pcie_check_hw_rf_kill(trans);
	if (hw_rfkill && !run_in_rfkill)