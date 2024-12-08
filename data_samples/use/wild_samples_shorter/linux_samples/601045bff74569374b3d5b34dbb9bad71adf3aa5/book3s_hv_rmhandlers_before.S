
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
BEGIN_FTR_SECTION
	b	skip_tm
END_FTR_SECTION_IFCLR(CPU_FTR_TM)

	/* Turn on TM/FP/VSX/VMX so we can restore them. */
	mfmsr	r5
	li	r6, MSR_TM >> 32
	sldi	r6, r6, 32
	or	r5, r5, r6
	ori	r5, r5, MSR_FP
	oris	r5, r5, (MSR_VEC | MSR_VSX)@h
	mtmsrd	r5

	/*
	 * The user may change these outside of a transaction, so they must
	 * always be context switched.
	 */
	ld	r5, VCPU_TFHAR(r4)
	ld	r6, VCPU_TFIAR(r4)
	ld	r7, VCPU_TEXASR(r4)
	mtspr	SPRN_TFHAR, r5
	mtspr	SPRN_TFIAR, r6
	mtspr	SPRN_TEXASR, r7

	ld	r5, VCPU_MSR(r4)
	rldicl. r5, r5, 64 - MSR_TS_S_LG, 62
	beq	skip_tm	/* TM not active in guest */

	/* Make sure the failure summary is set, otherwise we'll program check
	 * when we trechkpt.  It's possible that this might have been not set
	 * on a kvmppc_set_one_reg() call but we shouldn't let this crash the
	 * host.
	 */
	oris	r7, r7, (TEXASR_FS)@h
	mtspr	SPRN_TEXASR, r7

	/*
	 * We need to load up the checkpointed state for the guest.
	 * We need to do this early as it will blow away any GPRs, VSRs and
	 * some SPRs.
	 */

	mr	r31, r4
	addi	r3, r31, VCPU_FPRS_TM
	bl	load_fp_state
	addi	r3, r31, VCPU_VRS_TM
	bl	load_vr_state
	mr	r4, r31
	lwz	r7, VCPU_VRSAVE_TM(r4)
	mtspr	SPRN_VRSAVE, r7

	ld	r5, VCPU_LR_TM(r4)
	lwz	r6, VCPU_CR_TM(r4)
	ld	r7, VCPU_CTR_TM(r4)
	ld	r8, VCPU_AMR_TM(r4)
	ld	r9, VCPU_TAR_TM(r4)
	mtlr	r5
	mtcr	r6
	mtctr	r7
	mtspr	SPRN_AMR, r8
	mtspr	SPRN_TAR, r9

	/*
	 * Load up PPR and DSCR values but don't put them in the actual SPRs
	 * till the last moment to avoid running with userspace PPR and DSCR for
	 * too long.
	 */
	ld	r29, VCPU_DSCR_TM(r4)
	ld	r30, VCPU_PPR_TM(r4)

	std	r2, PACATMSCRATCH(r13) /* Save TOC */

	/* Clear the MSR RI since r1, r13 are all going to be foobar. */
	li	r5, 0
	mtmsrd	r5, 1

	/* Load GPRs r0-r28 */
	reg = 0
	.rept	29
	ld	reg, VCPU_GPRS_TM(reg)(r31)
	reg = reg + 1
	.endr

	mtspr	SPRN_DSCR, r29
	mtspr	SPRN_PPR, r30

	/* Load final GPRs */
	ld	29, VCPU_GPRS_TM(29)(r31)
	ld	30, VCPU_GPRS_TM(30)(r31)
	ld	31, VCPU_GPRS_TM(31)(r31)

	/* TM checkpointed state is now setup.  All GPRs are now volatile. */
	TRECHKPT

	/* Now let's get back the state we need. */
	HMT_MEDIUM
	GET_PACA(r13)
	ld	r29, HSTATE_DSCR(r13)
	mtspr	SPRN_DSCR, r29
	ld	r4, HSTATE_KVM_VCPU(r13)
	ld	r1, HSTATE_HOST_R1(r13)
	ld	r2, PACATMSCRATCH(r13)

	/* Set the MSR RI since we have our registers back. */
	li	r5, MSR_RI
	mtmsrd	r5, 1
skip_tm:
#endif

	/* Load guest PMU registers */
	/* R4 is live here (vcpu pointer) */
	/* Skip next section on POWER7 */
	b	8f
END_FTR_SECTION_IFCLR(CPU_FTR_ARCH_207S)
	/* Turn on TM so we can access TFHAR/TFIAR/TEXASR */
	mfmsr	r8
	li	r0, 1
	rldimi	r8, r0, MSR_TM_LG, 63-MSR_TM_LG
	mtmsrd	r8

	/* Load up POWER8-specific registers */
	ld	r5, VCPU_IAMR(r4)
	lwz	r6, VCPU_PSPB(r4)
	ld	r7, VCPU_FSCR(r4)

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
BEGIN_FTR_SECTION
	b	2f
END_FTR_SECTION_IFCLR(CPU_FTR_TM)
	/* Turn on TM. */
	mfmsr	r8
	li	r0, 1
	rldimi	r8, r0, MSR_TM_LG, 63-MSR_TM_LG
	mtmsrd	r8

	ld	r5, VCPU_MSR(r9)
	rldicl. r5, r5, 64 - MSR_TS_S_LG, 62
	beq	1f	/* TM not active in guest. */

	li	r3, TM_CAUSE_KVM_RESCHED

	/* Clear the MSR RI since r1, r13 are all going to be foobar. */
	li	r5, 0
	mtmsrd	r5, 1

	/* All GPRs are volatile at this point. */
	TRECLAIM(R3)

	/* Temporarily store r13 and r9 so we have some regs to play with */
	SET_SCRATCH0(r13)
	GET_PACA(r13)
	std	r9, PACATMSCRATCH(r13)
	ld	r9, HSTATE_KVM_VCPU(r13)

	/* Get a few more GPRs free. */
	std	r29, VCPU_GPRS_TM(29)(r9)
	std	r30, VCPU_GPRS_TM(30)(r9)
	std	r31, VCPU_GPRS_TM(31)(r9)

	/* Save away PPR and DSCR soon so don't run with user values. */
	mfspr	r31, SPRN_PPR
	HMT_MEDIUM
	mfspr	r30, SPRN_DSCR
	ld	r29, HSTATE_DSCR(r13)
	mtspr	SPRN_DSCR, r29

	/* Save all but r9, r13 & r29-r31 */
	reg = 0
	.rept	29
	.if (reg != 9) && (reg != 13)
	std	reg, VCPU_GPRS_TM(reg)(r9)
	.endif
	reg = reg + 1
	.endr
	/* ... now save r13 */
	GET_SCRATCH0(r4)
	std	r4, VCPU_GPRS_TM(13)(r9)
	/* ... and save r9 */
	ld	r4, PACATMSCRATCH(r13)
	std	r4, VCPU_GPRS_TM(9)(r9)

	/* Reload stack pointer and TOC. */
	ld	r1, HSTATE_HOST_R1(r13)
	ld	r2, PACATOC(r13)

	/* Set MSR RI now we have r1 and r13 back. */
	li	r5, MSR_RI
	mtmsrd	r5, 1

	/* Save away checkpinted SPRs. */
	std	r31, VCPU_PPR_TM(r9)
	std	r30, VCPU_DSCR_TM(r9)
	mflr	r5
	mfcr	r6
	mfctr	r7
	mfspr	r8, SPRN_AMR
	mfspr	r10, SPRN_TAR
	std	r5, VCPU_LR_TM(r9)
	stw	r6, VCPU_CR_TM(r9)
	std	r7, VCPU_CTR_TM(r9)
	std	r8, VCPU_AMR_TM(r9)
	std	r10, VCPU_TAR_TM(r9)

	/* Restore r12 as trap number. */
	lwz	r12, VCPU_TRAP(r9)

	/* Save FP/VSX. */
	addi	r3, r9, VCPU_FPRS_TM
	bl	store_fp_state
	addi	r3, r9, VCPU_VRS_TM
	bl	store_vr_state
	mfspr	r6, SPRN_VRSAVE
	stw	r6, VCPU_VRSAVE_TM(r9)
1:
	/*
	 * We need to save these SPRs after the treclaim so that the software
	 * error code is recorded correctly in the TEXASR.  Also the user may
	 * change these outside of a transaction, so they must always be
	 * context switched.
	 */
	mfspr	r5, SPRN_TFHAR
	mfspr	r6, SPRN_TFIAR
	mfspr	r7, SPRN_TEXASR
	std	r5, VCPU_TFHAR(r9)
	std	r6, VCPU_TFIAR(r9)
	std	r7, VCPU_TEXASR(r9)
2:
#endif

	/* Increment yield count if they have a VPA */
	ld	r8, VCPU_VPA(r9)	/* do they have a VPA? */
	/* save FP state */
	bl	kvmppc_save_fp

	/*
	 * Set DEC to the smaller of DEC and HDEC, so that we wake
	 * no later than the end of our timeslice (HDEC interrupts
	 * don't wake us from nap).
	bl	kvmhv_accumulate_time
#endif

	/* load up FP state */
	bl	kvmppc_load_fp

	/* Restore guest decrementer */
	mr	r4,r31
	blr

/*
 * We come here if we get any exception or interrupt while we are
 * executing host real mode code while in guest MMU context.
 * For now just spin, but we should do something better.