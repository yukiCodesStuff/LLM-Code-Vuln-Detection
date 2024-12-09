	local_irq_enable();
	if (!current_set_polling_and_test()) {
		unsigned int loop_count = 0;
		u64 limit = TICK_NSEC;
		int i;

		for (i = 1; i < drv->state_count; i++) {
			if (drv->states[i].disabled || dev->states_usage[i].disable)