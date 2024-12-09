	OFFSET(TASK_STACK, task_struct, stack);
	OFFSET(TASK_TI, task_struct, thread_info);
	OFFSET(TASK_TI_FLAGS, task_struct, thread_info.flags);
	OFFSET(TASK_TI_PREEMPT_COUNT, task_struct, thread_info.preempt_count);
	OFFSET(TASK_TI_KERNEL_SP, task_struct, thread_info.kernel_sp);
	OFFSET(TASK_TI_USER_SP, task_struct, thread_info.user_sp);
	OFFSET(TASK_TI_CPU, task_struct, thread_info.cpu);
