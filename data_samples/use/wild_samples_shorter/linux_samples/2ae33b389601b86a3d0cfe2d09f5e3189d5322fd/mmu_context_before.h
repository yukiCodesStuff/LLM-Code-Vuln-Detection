#ifdef CONFIG_CPU_HAS_ASID

void check_and_switch_context(struct mm_struct *mm, struct task_struct *tsk);
#define init_new_context(tsk,mm)	({ mm->context.id = 0; })

#else	/* !CONFIG_CPU_HAS_ASID */

#ifdef CONFIG_MMU