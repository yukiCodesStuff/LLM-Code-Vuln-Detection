	LOAD_REG_ADDR(r11, dawr_force_enable)
	lbz	r11, 0(r11)
	cmpdi	r11, 0
	bne	3f
	li	r3, H_HARDWARE
	blr
3:
	/* Emulate H_SET_DABR/X on P8 for the sake of compat mode guests */
	rlwimi	r5, r4, 5, DAWRX_DR | DAWRX_DW
	rlwimi	r5, r4, 2, DAWRX_WT
	clrrdi	r4, r4, 3
	std	r4, VCPU_DAWR(r3)
	std	r5, VCPU_DAWRX(r3)
	/*
	 * If came in through the real mode hcall handler then it is necessary
	 * to write the registers since the return path won't. Otherwise it is
	 * sufficient to store then in the vcpu struct as they will be loaded
	 * next time the vcpu is run.
	 */
	mfmsr	r6
	andi.	r6, r6, MSR_DR		/* in real mode? */
	bne	4f
	mtspr	SPRN_DAWR, r4
	mtspr	SPRN_DAWRX, r5
4:	li	r3, 0
	blr

_GLOBAL(kvmppc_h_cede)		/* r3 = vcpu pointer, r11 = msr, r13 = paca */
	ori	r11,r11,MSR_EE