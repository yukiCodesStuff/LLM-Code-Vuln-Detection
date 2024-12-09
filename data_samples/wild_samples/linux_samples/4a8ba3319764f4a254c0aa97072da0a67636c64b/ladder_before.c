	if (unlikely(latency_req == 0)) {
		ladder_do_selection(ldev, last_idx, 0);
		return 0;
	}

	last_state = &ldev->states[last_idx];

	if (!(drv->states[last_idx].flags & CPUIDLE_FLAG_TIME_INVALID)) {
		last_residency = cpuidle_get_last_residency(dev) - \
					 drv->states[last_idx].exit_latency;
	}