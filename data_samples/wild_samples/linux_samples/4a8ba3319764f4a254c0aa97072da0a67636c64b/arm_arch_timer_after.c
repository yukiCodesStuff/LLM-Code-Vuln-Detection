{
	u64 start_count;

	/* Register the CP15 based counter if we have one */
	if (type & ARCH_CP15_TIMER) {
		if (IS_ENABLED(CONFIG_ARM64) || arch_timer_use_virtual)
			arch_timer_read_counter = arch_counter_get_cntvct;
		else
			arch_timer_read_counter = arch_counter_get_cntpct;
	} else {
		arch_timer_read_counter = arch_counter_get_cntvct_mem;

		/* If the clocksource name is "arch_sys_counter" the
		 * VDSO will attempt to read the CP15-based counter.
		 * Ensure this does not happen when CP15-based
		 * counter is not available.
		 */
		clocksource_counter.name = "arch_mem_counter";
	}

	start_count = arch_timer_read_counter();
	clocksource_register_hz(&clocksource_counter, arch_timer_rate);
	cyclecounter.mult = clocksource_counter.mult;
	cyclecounter.shift = clocksource_counter.shift;
	timecounter_init(&timecounter, &cyclecounter, start_count);

	/* 56 bits minimum, so we assume worst case rollover */
	sched_clock_register(arch_timer_read_counter, 56, arch_timer_rate);
}