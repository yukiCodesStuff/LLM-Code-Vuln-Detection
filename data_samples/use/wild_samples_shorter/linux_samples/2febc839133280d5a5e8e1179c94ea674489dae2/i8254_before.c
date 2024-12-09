		return;

	timer = &pit->pit_state.timer;
	if (hrtimer_cancel(timer))
		hrtimer_start_expires(timer, HRTIMER_MODE_ABS);
}

static void destroy_pit_timer(struct kvm_pit *pit)
{