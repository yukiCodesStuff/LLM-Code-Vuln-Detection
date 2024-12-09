        * using debugger IPI.
        */

	if (!kdump_in_progress())
		kexec_prepare_cpus();

	pr_debug("kexec: Starting switchover sequence.\n");
