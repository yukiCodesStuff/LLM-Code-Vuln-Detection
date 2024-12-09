{
	struct request_queue *q = req->q;
	unsigned long expiry;

	if (req->cmd_flags & REQ_NO_TIMEOUT)
		return;

	/* blk-mq has its own handler, so we don't need ->rq_timed_out_fn */
	if (!q->mq_ops && !q->rq_timed_out_fn)
		return;

	BUG_ON(!list_empty(&req->timeout_list));

	/*
	 * Some LLDs, like scsi, peek at the timeout to prevent a
	 * command from being retried forever.
	 */
	if (!req->timeout)
		req->timeout = q->rq_timeout;

	req->deadline = jiffies + req->timeout;
	if (!q->mq_ops)
		list_add_tail(&req->timeout_list, &req->q->timeout_list);

	/*
	 * If the timer isn't already pending or this timeout is earlier
	 * than an existing one, modify the timer. Round up to next nearest
	 * second.
	 */
	expiry = blk_rq_timeout(round_jiffies_up(req->deadline));

	if (!timer_pending(&q->timeout) ||
	    time_before(expiry, q->timeout.expires)) {
		unsigned long diff = q->timeout.expires - expiry;

		/*
		 * Due to added timer slack to group timers, the timer
		 * will often be a little in front of what we asked for.
		 * So apply some tolerance here too, otherwise we keep
		 * modifying the timer because expires for value X
		 * will be X + something.
		 */
		if (!timer_pending(&q->timeout) || (diff >= HZ / 2))
			mod_timer(&q->timeout, expiry);
	}