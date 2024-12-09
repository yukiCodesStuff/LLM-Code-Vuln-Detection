	REG_L x2,  PT_SP(sp)
	.endm

#if !IS_ENABLED(CONFIG_PREEMPT)
.set resume_kernel, restore_all
#endif

ENTRY(handle_exception)
	SAVE_ALL

	/*
	REG_L s0, PT_SSTATUS(sp)
	csrc sstatus, SR_SIE
	andi s0, s0, SR_SPP
	bnez s0, resume_kernel

resume_userspace:
	/* Interrupts must be disabled here so flags are checked atomically */
	REG_L s0, TASK_TI_FLAGS(tp) /* current_thread_info->flags */
	RESTORE_ALL
	sret

#if IS_ENABLED(CONFIG_PREEMPT)
resume_kernel:
	REG_L s0, TASK_TI_PREEMPT_COUNT(tp)
	bnez s0, restore_all
need_resched:
	REG_L s0, TASK_TI_FLAGS(tp)
	andi s0, s0, _TIF_NEED_RESCHED
	beqz s0, restore_all
	call preempt_schedule_irq
	j need_resched
#endif

work_pending:
	/* Enter slow path for supplementary processing */
	la ra, ret_from_exception
	andi s1, s0, _TIF_NEED_RESCHED