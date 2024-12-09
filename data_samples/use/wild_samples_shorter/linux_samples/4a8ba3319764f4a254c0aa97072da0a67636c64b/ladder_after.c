
	last_state = &ldev->states[last_idx];

	last_residency = cpuidle_get_last_residency(dev) - drv->states[last_idx].exit_latency;

	/* consider promotion */
	if (last_idx < drv->state_count - 1 &&
	    !drv->states[last_idx + 1].disabled &&