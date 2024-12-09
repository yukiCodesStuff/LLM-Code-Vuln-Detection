		return;

	timer = &pit->pit_state.timer;
	mutex_lock(&pit->pit_state.lock);
	if (hrtimer_cancel(timer))
		hrtimer_start_expires(timer, HRTIMER_MODE_ABS);
	mutex_unlock(&pit->pit_state.lock);
}

static void destroy_pit_timer(struct kvm_pit *pit)
{