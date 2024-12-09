	u32 inta_fh_msk = ~MSIX_FH_INT_CAUSES_DATA_QUEUE;
	u32 inta_fh, inta_hw;
	bool polling = false;
	bool sw_err;

	if (trans_pcie->shared_vec_mask & IWL_SHARED_IRQ_NON_RX)
		inta_fh_msk |= MSIX_FH_INT_CAUSES_Q0;

		wake_up(&trans_pcie->ucode_write_waitq);
	}

	if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_BZ)
		sw_err = inta_hw & MSIX_HW_INT_CAUSES_REG_SW_ERR_BZ;
	else
		sw_err = inta_hw & MSIX_HW_INT_CAUSES_REG_SW_ERR;

	/* Error detected by uCode */
	if ((inta_fh & MSIX_FH_INT_CAUSES_FH_ERR) || sw_err) {
		IWL_ERR(trans,
			"Microcode SW error detected. Restarting 0x%X.\n",
			inta_fh);
		isr_stats->sw++;