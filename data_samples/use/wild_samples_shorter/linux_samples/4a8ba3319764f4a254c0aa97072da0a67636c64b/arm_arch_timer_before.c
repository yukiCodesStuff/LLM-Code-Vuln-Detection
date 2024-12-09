
	/* Register the CP15 based counter if we have one */
	if (type & ARCH_CP15_TIMER) {
		if (arch_timer_use_virtual)
			arch_timer_read_counter = arch_counter_get_cntvct;
		else
			arch_timer_read_counter = arch_counter_get_cntpct;
	} else {