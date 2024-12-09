	REG_L x2,  PT_SP(sp)
	.endm

ENTRY(handle_exception)
	SAVE_ALL

	/*
	REG_L s0, PT_SSTATUS(sp)
	csrc sstatus, SR_SIE
	andi s0, s0, SR_SPP
	bnez s0, restore_all

resume_userspace:
	/* Interrupts must be disabled here so flags are checked atomically */
	REG_L s0, TASK_TI_FLAGS(tp) /* current_thread_info->flags */
	RESTORE_ALL
	sret

work_pending:
	/* Enter slow path for supplementary processing */
	la ra, ret_from_exception
	andi s1, s0, _TIF_NEED_RESCHED