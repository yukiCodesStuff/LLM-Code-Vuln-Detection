
	DEFINE(SZ_CALLEE_REGS, sizeof(struct callee_regs));
	DEFINE(SZ_PT_REGS, sizeof(struct pt_regs));

#ifdef CONFIG_ISA_ARCV2
	OFFSET(PT_r12, pt_regs, r12);
	OFFSET(PT_r30, pt_regs, r30);
#endif
#ifdef CONFIG_ARC_HAS_ACCL_REGS
	OFFSET(PT_r58, pt_regs, r58);
	OFFSET(PT_r59, pt_regs, r59);
#endif

	return 0;
}