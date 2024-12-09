
	cpu = all_cpu_data[policy->cpu];

	intel_pstate_get_min_max(cpu, &min, &max);

	limits.min_perf_pct = (policy->min * 100) / policy->cpuinfo.max_freq;
	limits.min_perf_pct = clamp_t(int, limits.min_perf_pct, 0 , 100);
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
module_exit(intel_pstate_exit);

static int __initdata no_load;

static int __init intel_pstate_init(void)
{
	int rc = 0;
	const struct x86_cpu_id *id;

	if (no_load)
		return -ENODEV;
	intel_pstate_sysfs_expose_params();
	return rc;
out:
	intel_pstate_exit();
	return -ENODEV;
}
device_initcall(intel_pstate_init);
