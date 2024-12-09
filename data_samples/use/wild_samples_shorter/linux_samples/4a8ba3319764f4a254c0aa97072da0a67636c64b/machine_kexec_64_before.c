        * using debugger IPI.
        */

	if (crashing_cpu == -1)
		kexec_prepare_cpus();

	pr_debug("kexec: Starting switchover sequence.\n");
