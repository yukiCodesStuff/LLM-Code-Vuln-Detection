
	eret
ENDPROC(__fpsimd_guest_restore)

ENTRY(__qcom_hyp_sanitize_btac_predictors)
	/**
	 * Call SMC64 with Silicon provider serviceID 23<<8 (0xc2001700)
	 * 0xC2000000-0xC200FFFF: assigned to SiP Service Calls
	 * b15-b0: contains SiP functionID
	 */
	movz    x0, #0x1700
	movk    x0, #0xc200, lsl #16
	smc     #0
	ret
ENDPROC(__qcom_hyp_sanitize_btac_predictors)