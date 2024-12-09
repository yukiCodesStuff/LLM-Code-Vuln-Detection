{
	unsigned i;

	for (i = 0; i < max_cpus; ++i)
		set_cpu_present(i, true);
}

void __init smp_init_cpus(void)
	pr_info("%s: Core Count = %d\n", __func__, ncpus);
	pr_info("%s: Core Id = %d\n", __func__, core_id);

	for (i = 0; i < ncpus; ++i)
		set_cpu_possible(i, true);
}

	int i;

#ifdef CONFIG_HOTPLUG_CPU
	cpu_start_id = cpu;
	system_flush_invalidate_dcache_range(
			(unsigned long)&cpu_start_id, sizeof(cpu_start_id));
#endif
	smp_call_function_single(0, mx_cpu_start, (void *)cpu, 1);

	for (i = 0; i < 2; ++i) {
			ccount = get_ccount();
		while (!ccount);

		cpu_start_ccount = ccount;

		while (time_before(jiffies, timeout)) {
			mb();
			if (!cpu_start_ccount)
				break;
		}

		if (cpu_start_ccount) {
			smp_call_function_single(0, mx_cpu_stop,
					(void *)cpu, 1);
			cpu_start_ccount = 0;
			return -EIO;
		}
	}
	return 0;
	pr_debug("%s: Calling wakeup_secondary(cpu:%d, idle:%p, sp: %08lx)\n",
			__func__, cpu, idle, start_info.stack);

	ret = boot_secondary(cpu, idle);
	if (ret == 0) {
		wait_for_completion_timeout(&cpu_running,
				msecs_to_jiffies(1000));
	unsigned long timeout = jiffies + msecs_to_jiffies(1000);
	while (time_before(jiffies, timeout)) {
		system_invalidate_dcache_range((unsigned long)&cpu_start_id,
				sizeof(cpu_start_id));
		if (cpu_start_id == -cpu) {
			platform_cpu_kill(cpu);
			return;
		}
	}