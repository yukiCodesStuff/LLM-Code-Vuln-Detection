
	/*
	 * Reset BCH here, too. We got failures otherwise :(
	 * See later BCH reset for explanation of MX23 handling
	 */
	ret = gpmi_reset_block(r->bch_regs, GPMI_IS_MX23(this));
	if (ret)
		goto err_out;

	/* Choose NAND mode. */
	/*
	* Due to erratum #2847 of the MX23, the BCH cannot be soft reset on this
	* chip, otherwise it will lock up. So we skip resetting BCH on the MX23.
	* On the other hand, the MX28 needs the reset, because one case has been
	* seen where the BCH produced ECC errors constantly after 10000
	* consecutive reboots. The latter case has not been seen on the MX23
	* yet, still we don't know if it could happen there as well.
	*/
	ret = gpmi_reset_block(r->bch_regs, GPMI_IS_MX23(this));
	if (ret)
		goto err_out;

	/* Configure layout 0. */