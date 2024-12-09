{
	unsigned long usermsr;

	if (!tsk->thread.regs)
		return;

	usermsr = tsk->thread.regs->msr;

	if ((usermsr & msr_all_available) == 0)
		return;

	msr_check_and_set(msr_all_available);
	check_if_tm_restore_required(tsk);

	WARN_ON((usermsr & MSR_VSX) && !((usermsr & MSR_FP) && (usermsr & MSR_VEC)));

#ifdef CONFIG_PPC_FPU
	if (usermsr & MSR_FP)
		__giveup_fpu(tsk);
#endif
#ifdef CONFIG_ALTIVEC
	if (usermsr & MSR_VEC)
		__giveup_altivec(tsk);
#endif
#ifdef CONFIG_SPE
	if (usermsr & MSR_SPE)
		__giveup_spe(tsk);
#endif

	msr_check_and_clear(msr_all_available);
}
EXPORT_SYMBOL(giveup_all);

/*
 * The exception exit path calls restore_math() with interrupts hard disabled
 * but the soft irq state not "reconciled". ftrace code that calls
 * local_irq_save/restore causes warnings.
 *
 * Rather than complicate the exit path, just don't trace restore_math. This
 * could be done by having ftrace entry code check for this un-reconciled
 * condition where MSR[EE]=0 and PACA_IRQ_HARD_DIS is not set, and
 * temporarily fix it up for the duration of the ftrace call.
 */
void notrace restore_math(struct pt_regs *regs)
{
	unsigned long msr;

	if (!MSR_TM_ACTIVE(regs->msr) &&
		!current->thread.load_fp && !loadvec(current->thread))
		return;

	msr = regs->msr;
	msr_check_and_set(msr_all_available);

	/*
	 * Only reload if the bit is not set in the user MSR, the bit BEING set
	 * indicates that the registers are hot
	 */
	if ((!(msr & MSR_FP)) && restore_fp(current))
		msr |= MSR_FP | current->thread.fpexc_mode;

	if ((!(msr & MSR_VEC)) && restore_altivec(current))
		msr |= MSR_VEC;

	if ((msr & (MSR_FP | MSR_VEC)) == (MSR_FP | MSR_VEC) &&
			restore_vsx(current)) {
		msr |= MSR_VSX;
	}