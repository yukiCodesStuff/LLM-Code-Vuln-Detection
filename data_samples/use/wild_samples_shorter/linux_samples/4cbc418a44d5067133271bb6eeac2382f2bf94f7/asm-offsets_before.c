
	DEFINE(SZ_CALLEE_REGS, sizeof(struct callee_regs));
	DEFINE(SZ_PT_REGS, sizeof(struct pt_regs));
	DEFINE(PT_user_r25, offsetof(struct pt_regs, user_r25));

	return 0;
}