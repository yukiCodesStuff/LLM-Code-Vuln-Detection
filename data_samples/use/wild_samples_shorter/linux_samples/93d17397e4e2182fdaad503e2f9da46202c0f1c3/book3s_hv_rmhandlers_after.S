	/* save FP state */
	bl	kvmppc_save_fp

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
BEGIN_FTR_SECTION
	ld	r9, HSTATE_KVM_VCPU(r13)
	bl	kvmppc_save_tm
END_FTR_SECTION_IFSET(CPU_FTR_TM)
#endif

	/*
	 * Set DEC to the smaller of DEC and HDEC, so that we wake
	 * no later than the end of our timeslice (HDEC interrupts
	 * don't wake us from nap).
	bl	kvmhv_accumulate_time
#endif

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
BEGIN_FTR_SECTION
	bl	kvmppc_restore_tm
END_FTR_SECTION_IFSET(CPU_FTR_TM)
#endif

	/* load up FP state */
	bl	kvmppc_load_fp

	/* Restore guest decrementer */