{
	struct msix_entry *entry = dev_id;
	struct iwl_trans_pcie *trans_pcie = iwl_pcie_get_trans_pcie(entry);
	struct iwl_trans *trans = trans_pcie->trans;
	struct isr_statistics *isr_stats = &trans_pcie->isr_stats;
	u32 inta_fh_msk = ~MSIX_FH_INT_CAUSES_DATA_QUEUE;
	u32 inta_fh, inta_hw;
	bool polling = false;

	if (trans_pcie->shared_vec_mask & IWL_SHARED_IRQ_NON_RX)
		inta_fh_msk |= MSIX_FH_INT_CAUSES_Q0;

	if (trans_pcie->shared_vec_mask & IWL_SHARED_IRQ_FIRST_RSS)
		inta_fh_msk |= MSIX_FH_INT_CAUSES_Q1;

	lock_map_acquire(&trans->sync_cmd_lockdep_map);

	spin_lock_bh(&trans_pcie->irq_lock);
	inta_fh = iwl_read32(trans, CSR_MSIX_FH_INT_CAUSES_AD);
	inta_hw = iwl_read32(trans, CSR_MSIX_HW_INT_CAUSES_AD);
	/*
	 * Clear causes registers to avoid being handling the same cause.
	 */
	iwl_write32(trans, CSR_MSIX_FH_INT_CAUSES_AD, inta_fh & inta_fh_msk);
	iwl_write32(trans, CSR_MSIX_HW_INT_CAUSES_AD, inta_hw);
	spin_unlock_bh(&trans_pcie->irq_lock);

	trace_iwlwifi_dev_irq_msix(trans->dev, entry, true, inta_fh, inta_hw);

	if (unlikely(!(inta_fh | inta_hw))) {
		IWL_DEBUG_ISR(trans, "Ignore interrupt, inta == 0\n");
		lock_map_release(&trans->sync_cmd_lockdep_map);
		return IRQ_NONE;
	}
	if (inta_fh & MSIX_FH_INT_CAUSES_D2S_CH0_NUM) {
		IWL_DEBUG_ISR(trans, "uCode load interrupt\n");
		isr_stats->tx++;
		/*
		 * Wake up uCode load routine,
		 * now that load is complete
		 */
		trans_pcie->ucode_write_complete = true;
		wake_up(&trans_pcie->ucode_write_waitq);
	}

	/* Error detected by uCode */
	if ((inta_fh & MSIX_FH_INT_CAUSES_FH_ERR) ||
	    (inta_hw & MSIX_HW_INT_CAUSES_REG_SW_ERR)) {
		IWL_ERR(trans,
			"Microcode SW error detected. Restarting 0x%X.\n",
			inta_fh);
		isr_stats->sw++;
		/* during FW reset flow report errors from there */
		if (trans_pcie->fw_reset_state == FW_RESET_REQUESTED) {
			trans_pcie->fw_reset_state = FW_RESET_ERROR;
			wake_up(&trans_pcie->fw_reset_waitq);
		} else {
			iwl_pcie_irq_handle_error(trans);
		}
	}