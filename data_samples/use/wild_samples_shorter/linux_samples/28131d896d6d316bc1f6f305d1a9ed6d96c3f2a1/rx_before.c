	u32 inta_fh_msk = ~MSIX_FH_INT_CAUSES_DATA_QUEUE;
	u32 inta_fh, inta_hw;
	bool polling = false;

	if (trans_pcie->shared_vec_mask & IWL_SHARED_IRQ_NON_RX)
		inta_fh_msk |= MSIX_FH_INT_CAUSES_Q0;

		wake_up(&trans_pcie->ucode_write_waitq);
	}

	/* Error detected by uCode */
	if ((inta_fh & MSIX_FH_INT_CAUSES_FH_ERR) ||
	    (inta_hw & MSIX_HW_INT_CAUSES_REG_SW_ERR)) {
		IWL_ERR(trans,
			"Microcode SW error detected. Restarting 0x%X.\n",
			inta_fh);
		isr_stats->sw++;