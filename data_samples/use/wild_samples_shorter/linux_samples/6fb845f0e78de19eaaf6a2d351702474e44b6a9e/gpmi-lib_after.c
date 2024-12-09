
	/*
	 * Reset BCH here, too. We got failures otherwise :(
	 * See later BCH reset for explanation of MX23 and MX28 handling
	 */
	ret = gpmi_reset_block(r->bch_regs,
			       GPMI_IS_MX23(this) || GPMI_IS_MX28(this));
	if (ret)
		goto err_out;

	/* Choose NAND mode. */
	/*
	* Due to erratum #2847 of the MX23, the BCH cannot be soft reset on this
	* chip, otherwise it will lock up. So we skip resetting BCH on the MX23.
	* and MX28.
	*/
	ret = gpmi_reset_block(r->bch_regs,
			       GPMI_IS_MX23(this) || GPMI_IS_MX28(this));
	if (ret)
		goto err_out;

	/* Configure layout 0. */