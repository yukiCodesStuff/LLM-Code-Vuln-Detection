	if (events->overflow) {
		events->overflow = 0;
		return -EOVERFLOW;
	}

	if (events->eventw == events->eventr) {
		int ret;

		if (flags & O_NONBLOCK)
			return -EWOULDBLOCK;

		ret = wait_event_interruptible(events->wait_queue,
					       dvb_frontend_test_event(fepriv, events));

		if (ret < 0)
			return ret;
	}