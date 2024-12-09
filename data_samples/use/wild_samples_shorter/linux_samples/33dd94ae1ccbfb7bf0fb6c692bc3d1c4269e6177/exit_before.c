	if (unlikely(!tsk->pid))
		panic("Attempted to kill the idle task!");

	tracehook_report_exit(&code);

	validate_creds_for_do_exit(tsk);
