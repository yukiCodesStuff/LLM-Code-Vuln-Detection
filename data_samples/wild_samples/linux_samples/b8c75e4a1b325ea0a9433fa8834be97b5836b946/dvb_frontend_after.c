	if (events->overflow) {
		events->overflow = 0;
		return -EOVERFLOW;
	}

	if (events->eventw == events->eventr) {
		struct wait_queue_entry wait;
		int ret = 0;

		if (flags & O_NONBLOCK)
			return -EWOULDBLOCK;

		init_waitqueue_entry(&wait, current);
		add_wait_queue(&events->wait_queue, &wait);
		while (!dvb_frontend_test_event(fepriv, events)) {
			wait_woken(&wait, TASK_INTERRUPTIBLE, 0);
			if (signal_pending(current)) {
				ret = -ERESTARTSYS;
				break;
			}
		}
		remove_wait_queue(&events->wait_queue, &wait);
		if (ret < 0)
			return ret;
	}