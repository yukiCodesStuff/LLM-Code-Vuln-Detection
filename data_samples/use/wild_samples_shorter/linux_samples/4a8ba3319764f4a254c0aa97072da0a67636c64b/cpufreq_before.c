	/* Don't start any governor operations if we are entering suspend */
	if (cpufreq_suspended)
		return 0;

	if (policy->governor->max_transition_latency &&
	    policy->cpuinfo.transition_latency >
	    policy->governor->max_transition_latency) {