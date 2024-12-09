	SAVE_4GPRS(3, r11);						     \
	SAVE_2GPRS(7, r11)

.macro SYSCALL_ENTRY trapno intno srr1
	mfspr	r10, SPRN_SPRG_THREAD
#ifdef CONFIG_KVM_BOOKE_HV
BEGIN_FTR_SECTION
	mtspr	SPRN_SPRG_WSCRATCH0, r10
	mfspr	r11, SPRN_SRR1
	mtocrf	0x80, r11	/* check MSR[GS] without clobbering reg */
	bf	3, 1975f
	b	kvmppc_handler_\intno\()_\srr1
1975:
	mr	r12, r13
	lwz	r13, THREAD_NORMSAVE(2)(r10)
FTR_SECTION_ELSE
	tophys(r11,r11)
	addi	r11,r11,global_dbcr0@l
#ifdef CONFIG_SMP
	lwz	r10, TASK_CPU(r2)
	slwi	r10, r10, 3
	add	r11, r11, r10
#endif
	lwz	r12,0(r11)
	mtspr	SPRN_DBCR0,r12
	lwz	r12,4(r11)