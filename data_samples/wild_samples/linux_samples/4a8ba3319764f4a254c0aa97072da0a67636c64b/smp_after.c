{
	unsigned int cpu = smp_processor_id();
	int i, base;

	atomic_inc(&init_mm.mm_count);
	current->active_mm = &init_mm;

	smp_store_cpu_info(cpu);
	set_dec(tb_ticks_per_jiffy);
	preempt_disable();
	cpu_callin_map[cpu] = 1;

	if (smp_ops->setup_cpu)
		smp_ops->setup_cpu(cpu);
	if (smp_ops->take_timebase)
		smp_ops->take_timebase();

	secondary_cpu_time_init();

#ifdef CONFIG_PPC64
	if (system_state == SYSTEM_RUNNING)
		vdso_data->processorCount++;

	vdso_getcpu_init();
#endif
	/* Update sibling maps */
	base = cpu_first_thread_sibling(cpu);
	for (i = 0; i < threads_per_core; i++) {
		if (cpu_is_offline(base + i) && (cpu != base + i))
			continue;
		cpumask_set_cpu(cpu, cpu_sibling_mask(base + i));
		cpumask_set_cpu(base + i, cpu_sibling_mask(cpu));

		/* cpu_core_map should be a superset of
		 * cpu_sibling_map even if we don't have cache
		 * information, so update the former here, too.
		 */
		cpumask_set_cpu(cpu, cpu_core_mask(base + i));
		cpumask_set_cpu(base + i, cpu_core_mask(cpu));
	}
	for (i = 0; i < threads_per_core; i++) {
		if (cpu_is_offline(base + i) && (cpu != base + i))
			continue;
		cpumask_set_cpu(cpu, cpu_sibling_mask(base + i));
		cpumask_set_cpu(base + i, cpu_sibling_mask(cpu));

		/* cpu_core_map should be a superset of
		 * cpu_sibling_map even if we don't have cache
		 * information, so update the former here, too.
		 */
		cpumask_set_cpu(cpu, cpu_core_mask(base + i));
		cpumask_set_cpu(base + i, cpu_core_mask(cpu));
	}
	traverse_core_siblings(cpu, true);

	set_numa_node(numa_cpu_lookup_table[cpu]);
	set_numa_mem(local_memory_node(numa_cpu_lookup_table[cpu]));

	smp_wmb();
	notify_cpu_starting(cpu);
	set_cpu_online(cpu, true);

	local_irq_enable();

	cpu_startup_entry(CPUHP_ONLINE);

	BUG();
}

int setup_profiling_timer(unsigned int multiplier)
{
	return 0;
}

#ifdef CONFIG_SCHED_SMT
/* cpumask of CPUs with asymetric SMT dependancy */
static int powerpc_smt_flags(void)
{
	int flags = SD_SHARE_CPUCAPACITY | SD_SHARE_PKG_RESOURCES;

	if (cpu_has_feature(CPU_FTR_ASYM_SMT)) {
		printk_once(KERN_INFO "Enabling Asymmetric SMT scheduling\n");
		flags |= SD_ASYM_PACKING;
	}