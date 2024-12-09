	 * power state and occurrence of the wakeup event.
	 *
	 * If the entered idle state didn't support residency measurements,
	 * we are basically lost in the dark how much time passed.
	 * As a compromise, assume we slept for the whole expected time.
	 *
	 * Any measured amount of time will include the exit latency.
	 * Since we are interested in when the wakeup begun, not when it
	 * was completed, we must subtract the exit latency. However, if
	 * the measured amount of time is less than the exit latency,
	 * assume the state was never reached and the exit latency is 0.
	 */
	if (unlikely(target->flags & CPUIDLE_FLAG_TIME_INVALID)) {
		/* Use timer value as is */
		measured_us = data->next_timer_us;

	} else {
		/* Use measured value */
		measured_us = cpuidle_get_last_residency(dev);

		/* Deduct exit latency */
		if (measured_us > target->exit_latency)
			measured_us -= target->exit_latency;

		/* Make sure our coefficients do not exceed unity */
		if (measured_us > data->next_timer_us)
			measured_us = data->next_timer_us;
	}

	/* Update our correction ratio */
	new_factor = data->correction_factor[data->bucket];
	new_factor -= new_factor / DECAY;