}

#ifdef CONFIG_SCHED_SMT

static inline void set_idle_cores(int cpu, int val)
{
	struct sched_domain_shared *sds;