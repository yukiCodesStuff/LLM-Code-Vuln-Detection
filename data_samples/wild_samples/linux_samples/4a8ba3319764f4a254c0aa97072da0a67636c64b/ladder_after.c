	if (unlikely(latency_req == 0)) {
		ladder_do_selection(ldev, last_idx, 0);
		return 0;
	}

	last_state = &ldev->states[last_idx];

	last_residency = cpuidle_get_last_residency(dev) - drv->states[last_idx].exit_latency;

	/* consider promotion */
	if (last_idx < drv->state_count - 1 &&
	    !drv->states[last_idx + 1].disabled &&
	    !dev->states_usage[last_idx + 1].disable &&
	    last_residency > last_state->threshold.promotion_time &&
	    drv->states[last_idx + 1].exit_latency <= latency_req) {
		last_state->stats.promotion_count++;
		last_state->stats.demotion_count = 0;
		if (last_state->stats.promotion_count >= last_state->threshold.promotion_count) {
			ladder_do_selection(ldev, last_idx, last_idx + 1);
			return last_idx + 1;
		}
	}