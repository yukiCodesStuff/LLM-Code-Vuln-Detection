{
	int ret = 0;

	IWL_DEBUG_INFO(trans, "Init card's basic functions\n");

	/*
	 * Use "set_bit" below rather than "write", to preserve any hardware
	 * bits already set by default after reset.
	 */

	/*
	 * Disable L0s without affecting L1;
	 * don't wait for ICH L0s (ICH bug W/A)
	 */
	iwl_set_bit(trans, CSR_GIO_CHICKEN_BITS,
		    CSR_GIO_CHICKEN_BITS_REG_BIT_L1A_NO_L0S_RX);

	/* Set FH wait threshold to maximum (HW error during stress W/A) */
	iwl_set_bit(trans, CSR_DBG_HPET_MEM_REG, CSR_DBG_HPET_MEM_REG_VAL);

	/*
	 * Enable HAP INTA (interrupt from management bus) to
	 * wake device's PCI Express link L1a -> L0s
	 */
	iwl_set_bit(trans, CSR_HW_IF_CONFIG_REG,
		    CSR_HW_IF_CONFIG_REG_BIT_HAP_WAKE_L1A);

	iwl_pcie_apm_config(trans);

	ret = iwl_finish_nic_init(trans);
	if (ret)
		return ret;

	set_bit(STATUS_DEVICE_ENABLED, &trans->status);

	return 0;
}

static void iwl_pcie_gen2_apm_stop(struct iwl_trans *trans, bool op_mode_leave)
{
	IWL_DEBUG_INFO(trans, "Stop card, put in low power state\n");

	if (op_mode_leave) {
		if (!test_bit(STATUS_DEVICE_ENABLED, &trans->status))
			iwl_pcie_gen2_apm_init(trans);

		/* inform ME that we are leaving */
		iwl_set_bit(trans, CSR_DBG_LINK_PWR_MGMT_REG,
			    CSR_RESET_LINK_PWR_MGMT_DISABLED);
		iwl_set_bit(trans, CSR_HW_IF_CONFIG_REG,
			    CSR_HW_IF_CONFIG_REG_PREPARE |
			    CSR_HW_IF_CONFIG_REG_ENABLE_PME);
		mdelay(1);
		iwl_clear_bit(trans, CSR_DBG_LINK_PWR_MGMT_REG,
			      CSR_RESET_LINK_PWR_MGMT_DISABLED);
		mdelay(5);
	}
{
	struct iwl_trans_pcie *trans_pcie = IWL_TRANS_GET_PCIE_TRANS(trans);

	lockdep_assert_held(&trans_pcie->mutex);

	if (trans_pcie->is_down)
		return;

	if (trans->state >= IWL_TRANS_FW_STARTED)
		if (trans_pcie->fw_reset_handshake)
			iwl_trans_pcie_fw_reset_handshake(trans);

	trans_pcie->is_down = true;

	/* tell the device to stop sending interrupts */
	iwl_disable_interrupts(trans);

	/* device going down, Stop using ICT table */
	iwl_pcie_disable_ict(trans);

	/*
	 * If a HW restart happens during firmware loading,
	 * then the firmware loading might call this function
	 * and later it might be called again due to the
	 * restart. So don't process again if the device is
	 * already dead.
	 */
	if (test_and_clear_bit(STATUS_DEVICE_ENABLED, &trans->status)) {
		IWL_DEBUG_INFO(trans,
			       "DEVICE_ENABLED bit was set and is now cleared\n");
		iwl_txq_gen2_tx_free(trans);
		iwl_pcie_rx_stop(trans);
	}
	if (test_and_clear_bit(STATUS_DEVICE_ENABLED, &trans->status)) {
		IWL_DEBUG_INFO(trans,
			       "DEVICE_ENABLED bit was set and is now cleared\n");
		iwl_txq_gen2_tx_free(trans);
		iwl_pcie_rx_stop(trans);
	}

	iwl_pcie_ctxt_info_free_paging(trans);
	if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_AX210)
		iwl_pcie_ctxt_info_gen3_free(trans, false);
	else
		iwl_pcie_ctxt_info_free(trans);

	/* Stop the device, and put it in low power state */
	iwl_pcie_gen2_apm_stop(trans, false);

	iwl_trans_sw_reset(trans);

	/*
	 * Upon stop, the IVAR table gets erased, so msi-x won't
	 * work. This causes a bug in RF-KILL flows, since the interrupt
	 * that enables radio won't fire on the correct irq, and the
	 * driver won't be able to handle the interrupt.
	 * Configure the IVAR table again after reset.
	 */
	iwl_pcie_conf_msix_hw(trans_pcie);

	/*
	 * Upon stop, the APM issues an interrupt if HW RF kill is set.
	 * This is a bug in certain verions of the hardware.
	 * Certain devices also keep sending HW RF kill interrupt all
	 * the time, unless the interrupt is ACKed even if the interrupt
	 * should be masked. Re-ACK all the interrupts here.
	 */
	iwl_disable_interrupts(trans);

	/* clear all status bits */
	clear_bit(STATUS_SYNC_HCMD_ACTIVE, &trans->status);
	clear_bit(STATUS_INT_ENABLED, &trans->status);
	clear_bit(STATUS_TPOWER_PMI, &trans->status);

	/*
	 * Even if we stop the HW, we still want the RF kill
	 * interrupt
	 */
	iwl_enable_rfkill_int(trans);

	/* re-take ownership to prevent other users from stealing the device */
	iwl_pcie_prepare_card_hw(trans);
}

void iwl_trans_pcie_gen2_stop_device(struct iwl_trans *trans)
{
	struct iwl_trans_pcie *trans_pcie = IWL_TRANS_GET_PCIE_TRANS(trans);
	bool was_in_rfkill;

	iwl_op_mode_time_point(trans->op_mode,
			       IWL_FW_INI_TIME_POINT_HOST_DEVICE_DISABLE,
			       NULL);

	mutex_lock(&trans_pcie->mutex);
	trans_pcie->opmode_down = true;
	was_in_rfkill = test_bit(STATUS_RFKILL_OPMODE, &trans->status);
	_iwl_trans_pcie_gen2_stop_device(trans);
	iwl_trans_pcie_handle_stop_rfkill(trans, was_in_rfkill);
	mutex_unlock(&trans_pcie->mutex);
}

static int iwl_pcie_gen2_nic_init(struct iwl_trans *trans)
{
	struct iwl_trans_pcie *trans_pcie = IWL_TRANS_GET_PCIE_TRANS(trans);
	int queue_size = max_t(u32, IWL_CMD_QUEUE_SIZE,
			       trans->cfg->min_txq_size);

	/* TODO: most of the logic can be removed in A0 - but not in Z0 */
	spin_lock_bh(&trans_pcie->irq_lock);
	iwl_pcie_gen2_apm_init(trans);
	spin_unlock_bh(&trans_pcie->irq_lock);

	iwl_op_mode_nic_config(trans->op_mode);

	/* Allocate the RX queue, or reset if it is already allocated */
	if (iwl_pcie_gen2_rx_init(trans))
		return -ENOMEM;

	/* Allocate or reset and init all Tx and Command queues */
	if (iwl_txq_gen2_init(trans, trans->txqs.cmd.q_id, queue_size))
		return -ENOMEM;

	/* enable shadow regs in HW */
	iwl_set_bit(trans, CSR_MAC_SHADOW_REG_CTRL, 0x800FFFFF);
	IWL_DEBUG_INFO(trans, "Enabling shadow registers in device\n");

	return 0;
}

static void iwl_pcie_get_rf_name(struct iwl_trans *trans)
{
	struct iwl_trans_pcie *trans_pcie = IWL_TRANS_GET_PCIE_TRANS(trans);
	char *buf = trans_pcie->rf_name;
	size_t buflen = sizeof(trans_pcie->rf_name);
	size_t pos;
	u32 version;

	if (buf[0])
		return;

	switch (CSR_HW_RFID_TYPE(trans->hw_rf_id)) {
	case CSR_HW_RFID_TYPE(CSR_HW_RF_ID_TYPE_JF):
		pos = scnprintf(buf, buflen, "JF");
		break;
	case CSR_HW_RFID_TYPE(CSR_HW_RF_ID_TYPE_GF):
		pos = scnprintf(buf, buflen, "GF");
		break;
	case CSR_HW_RFID_TYPE(CSR_HW_RF_ID_TYPE_GF4):
		pos = scnprintf(buf, buflen, "GF4");
		break;
	case CSR_HW_RFID_TYPE(CSR_HW_RF_ID_TYPE_HR):
		pos = scnprintf(buf, buflen, "HR");
		break;
	case CSR_HW_RFID_TYPE(CSR_HW_RF_ID_TYPE_HR1):
		pos = scnprintf(buf, buflen, "HR1");
		break;
	case CSR_HW_RFID_TYPE(CSR_HW_RF_ID_TYPE_HRCDB):
		pos = scnprintf(buf, buflen, "HRCDB");
		break;
	default:
		return;
	}
	if (ret) {
		IWL_ERR(trans, "Unable to init nic\n");
		goto out;
	}

	if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_AX210)
		ret = iwl_pcie_ctxt_info_gen3_init(trans, fw);
	else
		ret = iwl_pcie_ctxt_info_init(trans, fw);
	if (ret)
		goto out;

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
		ret = -ERFKILL;

out:
	mutex_unlock(&trans_pcie->mutex);
	return ret;
}