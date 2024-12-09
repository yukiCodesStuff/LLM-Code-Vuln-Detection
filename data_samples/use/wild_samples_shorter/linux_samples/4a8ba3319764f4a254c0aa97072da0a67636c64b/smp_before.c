	smp_store_cpu_info(cpu);
	set_dec(tb_ticks_per_jiffy);
	preempt_disable();

	if (smp_ops->setup_cpu)
		smp_ops->setup_cpu(cpu);
	if (smp_ops->take_timebase)
	notify_cpu_starting(cpu);
	set_cpu_online(cpu, true);

	/*
	 * CPU must be marked active and online before we signal back to the
	 * master, because the scheduler needs to see the cpu_online and
	 * cpu_active bits set.
	 */
	smp_wmb();
	cpu_callin_map[cpu] = 1;

	local_irq_enable();

	cpu_startup_entry(CPUHP_ONLINE);
