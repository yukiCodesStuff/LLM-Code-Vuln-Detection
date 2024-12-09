	}
}

#else
static inline void check_if_tm_restore_required(struct task_struct *tsk) { }
#endif /* CONFIG_PPC_TRANSACTIONAL_MEM */

bool strict_msr_control;
EXPORT_SYMBOL(strict_msr_control);

static int restore_fp(struct task_struct *tsk)
{
	if (tsk->thread.load_fp) {
		load_fp_state(&current->thread.fp_state);
		current->thread.load_fp++;
		return 1;
	}

static int restore_altivec(struct task_struct *tsk)
{
	if (cpu_has_feature(CPU_FTR_ALTIVEC) && (tsk->thread.load_vec)) {
		load_vr_state(&tsk->thread.vr_state);
		tsk->thread.used_vr = 1;
		tsk->thread.load_vec++;
