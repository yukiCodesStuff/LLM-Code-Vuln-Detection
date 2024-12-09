	struct rq *rq = cpu_rq(cpu);
	struct rq_flags rf;

#ifdef CONFIG_SCHED_SMT
	/*
	 * The sched_smt_present static key needs to be evaluated on every
	 * hotplug event because at boot time SMT might be disabled when
	 * the number of booted CPUs is limited.
	 *
	 * If then later a sibling gets hotplugged, then the key would stay
	 * off and SMT scheduling would never be functional.
	 */
	if (cpumask_weight(cpu_smt_mask(cpu)) > 1)
		static_branch_enable_cpuslocked(&sched_smt_present);
#endif
	set_cpu_active(cpu, true);

	if (sched_smp_initialized) {
		sched_domains_numa_masks_set(cpu);
}
#endif

void __init sched_init_smp(void)
{
	sched_init_numa();

	init_sched_rt_class();
	init_sched_dl_class();

	sched_smp_initialized = true;
}

static int __init migration_init(void)