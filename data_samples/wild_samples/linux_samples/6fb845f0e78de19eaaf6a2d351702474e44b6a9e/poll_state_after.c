	if (!current_set_polling_and_test()) {
		unsigned int loop_count = 0;
		u64 limit = TICK_NSEC;
		int i;

		for (i = 1; i < drv->state_count; i++) {
			if (drv->states[i].disabled || dev->states_usage[i].disable)
				continue;

			limit = (u64)drv->states[i].target_residency * NSEC_PER_USEC;
			break;
		}

		while (!need_resched()) {
			cpu_relax();
			if (loop_count++ < POLL_IDLE_RELAX_COUNT)
				continue;

			loop_count = 0;
			if (local_clock() - time_start > limit) {
				dev->poll_time_limit = true;
				break;
			}
		}
	}