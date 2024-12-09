
	last_state = &ldev->states[last_idx];

	if (!(drv->states[last_idx].flags & CPUIDLE_FLAG_TIME_INVALID)) {
		last_residency = cpuidle_get_last_residency(dev) - \
					 drv->states[last_idx].exit_latency;
	}
	else
		last_residency = last_state->threshold.promotion_time + 1;

	/* consider promotion */
	if (last_idx < drv->state_count - 1 &&
	    !drv->states[last_idx + 1].disabled &&