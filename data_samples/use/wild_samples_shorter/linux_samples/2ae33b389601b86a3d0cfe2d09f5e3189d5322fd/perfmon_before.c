	.mount    = pfmfs_mount,
	.kill_sb  = kill_anon_super,
};

DEFINE_PER_CPU(unsigned long, pfm_syst_info);
DEFINE_PER_CPU(struct task_struct *, pmu_owner);
DEFINE_PER_CPU(pfm_context_t  *, pmu_ctx);