{
	return PERF_SAMPLE_REGS_ABI_32;
}

void perf_get_regs_user(struct perf_regs *regs_user,
			struct pt_regs *regs,
			struct pt_regs *regs_user_copy)
{
	regs_user->regs = task_pt_regs(current);
	regs_user->abi = perf_reg_abi(current);
}
#else /* CONFIG_X86_64 */
#define REG_NOSUPPORT ((1ULL << PERF_REG_X86_DS) | \
		       (1ULL << PERF_REG_X86_ES) | \
		       (1ULL << PERF_REG_X86_FS) | \
		       (1ULL << PERF_REG_X86_GS))

int perf_reg_validate(u64 mask)
{
	if (!mask || mask & REG_RESERVED)
		return -EINVAL;

	if (mask & REG_NOSUPPORT)
		return -EINVAL;

	return 0;
}

u64 perf_reg_abi(struct task_struct *task)
{
	if (test_tsk_thread_flag(task, TIF_IA32))
		return PERF_SAMPLE_REGS_ABI_32;
	else
		return PERF_SAMPLE_REGS_ABI_64;
}

void perf_get_regs_user(struct perf_regs *regs_user,
			struct pt_regs *regs,
			struct pt_regs *regs_user_copy)
{
	struct pt_regs *user_regs = task_pt_regs(current);

	/*
	 * If we're in an NMI that interrupted task_pt_regs setup, then
	 * we can't sample user regs at all.  This check isn't really
	 * sufficient, though, as we could be in an NMI inside an interrupt
	 * that happened during task_pt_regs setup.
	 */
	if (regs->sp > (unsigned long)&user_regs->r11 &&
	    regs->sp <= (unsigned long)(user_regs + 1)) {
		regs_user->abi = PERF_SAMPLE_REGS_ABI_NONE;
		regs_user->regs = NULL;
		return;
	}
{
	if (test_tsk_thread_flag(task, TIF_IA32))
		return PERF_SAMPLE_REGS_ABI_32;
	else
		return PERF_SAMPLE_REGS_ABI_64;
}

void perf_get_regs_user(struct perf_regs *regs_user,
			struct pt_regs *regs,
			struct pt_regs *regs_user_copy)
{
	struct pt_regs *user_regs = task_pt_regs(current);

	/*
	 * If we're in an NMI that interrupted task_pt_regs setup, then
	 * we can't sample user regs at all.  This check isn't really
	 * sufficient, though, as we could be in an NMI inside an interrupt
	 * that happened during task_pt_regs setup.
	 */
	if (regs->sp > (unsigned long)&user_regs->r11 &&
	    regs->sp <= (unsigned long)(user_regs + 1)) {
		regs_user->abi = PERF_SAMPLE_REGS_ABI_NONE;
		regs_user->regs = NULL;
		return;
	}