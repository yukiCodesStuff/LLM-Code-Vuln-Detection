
	cpu = all_cpu_data[policy->cpu];

	if (!policy->cpuinfo.max_freq)
		return -ENODEV;

	intel_pstate_get_min_max(cpu, &min, &max);

	limits.min_perf_pct = (policy->min * 100) / policy->cpuinfo.max_freq;
	limits.min_perf_pct = clamp_t(int, limits.min_perf_pct, 0 , 100);
	.owner		= THIS_MODULE,
};

static int __initdata no_load;

static int __init intel_pstate_init(void)
{
	int cpu, rc = 0;
	const struct x86_cpu_id *id;

	if (no_load)
		return -ENODEV;
	intel_pstate_sysfs_expose_params();
	return rc;
out:
	get_online_cpus();
	for_each_online_cpu(cpu) {
		if (all_cpu_data[cpu]) {
			del_timer_sync(&all_cpu_data[cpu]->timer);
			kfree(all_cpu_data[cpu]);
		}
	}

	put_online_cpus();
	vfree(all_cpu_data);
	return -ENODEV;
}
device_initcall(intel_pstate_init);
