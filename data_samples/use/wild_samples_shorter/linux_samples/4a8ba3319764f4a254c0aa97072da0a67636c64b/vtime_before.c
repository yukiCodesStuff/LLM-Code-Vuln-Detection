	struct thread_info *ti = task_thread_info(tsk);
	u64 timer, system;

	WARN_ON_ONCE(!irqs_disabled());

	timer = S390_lowcore.last_update_timer;
	S390_lowcore.last_update_timer = get_vtimer();
	S390_lowcore.system_timer += timer - S390_lowcore.last_update_timer;
