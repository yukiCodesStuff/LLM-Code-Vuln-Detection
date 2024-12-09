{
	int ret;

	/* Only must be defined when default governor is known to have latency
	   restrictions, like e.g. conservative or ondemand.
	   That this is the case is already ensured in Kconfig
	*/
#ifdef CONFIG_CPU_FREQ_GOV_PERFORMANCE
	struct cpufreq_governor *gov = &cpufreq_gov_performance;
#else
	struct cpufreq_governor *gov = NULL;
#endif

	/* Don't start any governor operations if we are entering suspend */
	if (cpufreq_suspended)
		return 0;

	if (policy->governor->max_transition_latency &&
	    policy->cpuinfo.transition_latency >
	    policy->governor->max_transition_latency) {
		if (!gov)
			return -EINVAL;
		else {
			pr_warn("%s governor failed, too long transition latency of HW, fallback to %s governor\n",
				policy->governor->name, gov->name);
			policy->governor = gov;
		}
	}