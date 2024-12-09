	    !test_thread_flag(TIF_RESTORE_TM)) {
		tsk->thread.ckpt_regs.msr = tsk->thread.regs->msr;
		set_thread_flag(TIF_RESTORE_TM);
	}
}

#else
static inline void check_if_tm_restore_required(struct task_struct *tsk) { }
#endif /* CONFIG_PPC_TRANSACTIONAL_MEM */

bool strict_msr_control;
EXPORT_SYMBOL(strict_msr_control);

static int __init enable_strict_msr_control(char *str)
{
	strict_msr_control = true;
	pr_info("Enabling strict facility control\n");

	return 0;
}
early_param("ppc_strict_facility_enable", enable_strict_msr_control);

/* notrace because it's called by restore_math */
unsigned long notrace msr_check_and_set(unsigned long bits)
{
	unsigned long oldmsr = mfmsr();
	unsigned long newmsr;

	newmsr = oldmsr | bits;

#ifdef CONFIG_VSX
	if (cpu_has_feature(CPU_FTR_VSX) && (bits & MSR_FP))
		newmsr |= MSR_VSX;
#endif

	if (oldmsr != newmsr)
		mtmsr_isync(newmsr);

	return newmsr;
}
EXPORT_SYMBOL_GPL(msr_check_and_set);

/* notrace because it's called by restore_math */
void notrace __msr_check_and_clear(unsigned long bits)
{
	unsigned long oldmsr = mfmsr();
	unsigned long newmsr;

	newmsr = oldmsr & ~bits;

#ifdef CONFIG_VSX
	if (cpu_has_feature(CPU_FTR_VSX) && (bits & MSR_FP))
		newmsr &= ~MSR_VSX;
#endif

	if (oldmsr != newmsr)
		mtmsr_isync(newmsr);
}
EXPORT_SYMBOL(__msr_check_and_clear);

#ifdef CONFIG_PPC_FPU
static void __giveup_fpu(struct task_struct *tsk)
{
	unsigned long msr;

	save_fpu(tsk);
	msr = tsk->thread.regs->msr;
	msr &= ~(MSR_FP|MSR_FE0|MSR_FE1);
#ifdef CONFIG_VSX
	if (cpu_has_feature(CPU_FTR_VSX))
		msr &= ~MSR_VSX;
#endif
	tsk->thread.regs->msr = msr;
}

void giveup_fpu(struct task_struct *tsk)
{
	check_if_tm_restore_required(tsk);

	msr_check_and_set(MSR_FP);
	__giveup_fpu(tsk);
	msr_check_and_clear(MSR_FP);
}
EXPORT_SYMBOL(giveup_fpu);

/*
 * Make sure the floating-point register state in the
 * the thread_struct is up to date for task tsk.
 */
void flush_fp_to_thread(struct task_struct *tsk)
{
	if (tsk->thread.regs) {
		/*
		 * We need to disable preemption here because if we didn't,
		 * another process could get scheduled after the regs->msr
		 * test but before we have finished saving the FP registers
		 * to the thread_struct.  That process could take over the
		 * FPU, and then when we get scheduled again we would store
		 * bogus values for the remaining FP registers.
		 */
		preempt_disable();
		if (tsk->thread.regs->msr & MSR_FP) {
			/*
			 * This should only ever be called for current or
			 * for a stopped child process.  Since we save away
			 * the FP register state on context switch,
			 * there is something wrong if a stopped child appears
			 * to still have its FP state in the CPU registers.
			 */
			BUG_ON(tsk != current);
			giveup_fpu(tsk);
		}
	if (current->thread.regs && (current->thread.regs->msr & MSR_FP)) {
		check_if_tm_restore_required(current);
		/*
		 * If a thread has already been reclaimed then the
		 * checkpointed registers are on the CPU but have definitely
		 * been saved by the reclaim code. Don't need to and *cannot*
		 * giveup as this would save  to the 'live' structure not the
		 * checkpointed structure.
		 */
		if (!MSR_TM_ACTIVE(cpumsr) &&
		     MSR_TM_ACTIVE(current->thread.regs->msr))
			return;
		__giveup_fpu(current);
	}
}
EXPORT_SYMBOL(enable_kernel_fp);

static int restore_fp(struct task_struct *tsk)
{
	if (tsk->thread.load_fp) {
		load_fp_state(&current->thread.fp_state);
		current->thread.load_fp++;
		return 1;
	}
	return 0;
}
		if (tsk->thread.regs->msr & MSR_VEC) {
			BUG_ON(tsk != current);
			giveup_altivec(tsk);
		}
		preempt_enable();
	}
}
EXPORT_SYMBOL_GPL(flush_altivec_to_thread);

static int restore_altivec(struct task_struct *tsk)
{
	if (cpu_has_feature(CPU_FTR_ALTIVEC) && (tsk->thread.load_vec)) {
		load_vr_state(&tsk->thread.vr_state);
		tsk->thread.used_vr = 1;
		tsk->thread.load_vec++;

		return 1;
	}
	return 0;
}