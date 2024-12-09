{
	struct cpudata *cpu;
	int min, max;

	cpu = all_cpu_data[policy->cpu];

	intel_pstate_get_min_max(cpu, &min, &max);

	limits.min_perf_pct = (policy->min * 100) / policy->cpuinfo.max_freq;
	limits.min_perf_pct = clamp_t(int, limits.min_perf_pct, 0 , 100);
	limits.min_perf = div_fp(int_tofp(limits.min_perf_pct), int_tofp(100));

	limits.max_perf_pct = policy->max * 100 / policy->cpuinfo.max_freq;
	limits.max_perf_pct = clamp_t(int, limits.max_perf_pct, 0 , 100);
	limits.max_perf = div_fp(int_tofp(limits.max_perf_pct), int_tofp(100));

	if (policy->policy == CPUFREQ_POLICY_PERFORMANCE) {
		limits.min_perf_pct = 100;
		limits.min_perf = int_tofp(1);
		limits.max_perf_pct = 100;
		limits.max_perf = int_tofp(1);
		limits.no_turbo = 0;
	}
static struct cpufreq_driver intel_pstate_driver = {
	.flags		= CPUFREQ_CONST_LOOPS,
	.verify		= intel_pstate_verify_policy,
	.setpolicy	= intel_pstate_set_policy,
	.get		= intel_pstate_get,
	.init		= intel_pstate_cpu_init,
	.exit		= intel_pstate_cpu_exit,
	.name		= "intel_pstate",
	.owner		= THIS_MODULE,
};

static void intel_pstate_exit(void)
{
	int cpu;

	sysfs_remove_group(intel_pstate_kobject,
				&intel_pstate_attr_group);
	debugfs_remove_recursive(debugfs_parent);

	cpufreq_unregister_driver(&intel_pstate_driver);

	if (!all_cpu_data)
		return;

	get_online_cpus();
	for_each_online_cpu(cpu) {
		if (all_cpu_data[cpu]) {
			del_timer_sync(&all_cpu_data[cpu]->timer);
			kfree(all_cpu_data[cpu]);
		}
	}

	put_online_cpus();
	vfree(all_cpu_data);
}
{
	int rc = 0;
	const struct x86_cpu_id *id;

	if (no_load)
		return -ENODEV;

	id = x86_match_cpu(intel_pstate_cpu_ids);
	if (!id)
		return -ENODEV;

	pr_info("Intel P-state driver initializing.\n");

	all_cpu_data = vmalloc(sizeof(void *) * num_possible_cpus());
	if (!all_cpu_data)
		return -ENOMEM;
	memset(all_cpu_data, 0, sizeof(void *) * num_possible_cpus());

	rc = cpufreq_register_driver(&intel_pstate_driver);
	if (rc)
		goto out;

	intel_pstate_debug_expose_params();
	intel_pstate_sysfs_expose_params();
	return rc;
out:
	intel_pstate_exit();
	return -ENODEV;
}
device_initcall(intel_pstate_init);

static int __init intel_pstate_setup(char *str)
{
	if (!str)
		return -EINVAL;

	if (!strcmp(str, "disable"))
		no_load = 1;
	return 0;
}
early_param("intel_pstate", intel_pstate_setup);

MODULE_AUTHOR("Dirk Brandewie <dirk.j.brandewie@intel.com>");
MODULE_DESCRIPTION("'intel_pstate' - P state driver Intel Core processors");
MODULE_LICENSE("GPL");