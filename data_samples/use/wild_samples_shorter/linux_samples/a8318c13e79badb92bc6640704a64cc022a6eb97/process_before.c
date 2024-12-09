	}
}

static bool tm_active_with_fp(struct task_struct *tsk)
{
	return MSR_TM_ACTIVE(tsk->thread.regs->msr) &&
		(tsk->thread.ckpt_regs.msr & MSR_FP);
}

static bool tm_active_with_altivec(struct task_struct *tsk)
{
	return MSR_TM_ACTIVE(tsk->thread.regs->msr) &&
		(tsk->thread.ckpt_regs.msr & MSR_VEC);
}
#else
static inline void check_if_tm_restore_required(struct task_struct *tsk) { }
static inline bool tm_active_with_fp(struct task_struct *tsk) { return false; }
static inline bool tm_active_with_altivec(struct task_struct *tsk) { return false; }
#endif /* CONFIG_PPC_TRANSACTIONAL_MEM */

bool strict_msr_control;
EXPORT_SYMBOL(strict_msr_control);

static int restore_fp(struct task_struct *tsk)
{
	if (tsk->thread.load_fp || tm_active_with_fp(tsk)) {
		load_fp_state(&current->thread.fp_state);
		current->thread.load_fp++;
		return 1;
	}

static int restore_altivec(struct task_struct *tsk)
{
	if (cpu_has_feature(CPU_FTR_ALTIVEC) &&
		(tsk->thread.load_vec || tm_active_with_altivec(tsk))) {
		load_vr_state(&tsk->thread.vr_state);
		tsk->thread.used_vr = 1;
		tsk->thread.load_vec++;
