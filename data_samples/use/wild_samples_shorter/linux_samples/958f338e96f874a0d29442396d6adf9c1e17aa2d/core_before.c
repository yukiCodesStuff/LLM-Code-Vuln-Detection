	struct rq *rq = cpu_rq(cpu);
	struct rq_flags rf;

	set_cpu_active(cpu, true);

	if (sched_smp_initialized) {
		sched_domains_numa_masks_set(cpu);
}
#endif

#ifdef CONFIG_SCHED_SMT
DEFINE_STATIC_KEY_FALSE(sched_smt_present);

static void sched_init_smt(void)
{
	/*
	 * We've enumerated all CPUs and will assume that if any CPU
	 * has SMT siblings, CPU0 will too.
	 */
	if (cpumask_weight(cpu_smt_mask(0)) > 1)
		static_branch_enable(&sched_smt_present);
}
#else
static inline void sched_init_smt(void) { }
#endif

void __init sched_init_smp(void)
{
	sched_init_numa();

	init_sched_rt_class();
	init_sched_dl_class();

	sched_init_smt();

	sched_smp_initialized = true;
}

static int __init migration_init(void)