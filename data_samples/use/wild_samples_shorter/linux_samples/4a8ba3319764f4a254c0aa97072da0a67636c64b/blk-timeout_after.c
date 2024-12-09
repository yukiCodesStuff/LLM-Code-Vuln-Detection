	struct request_queue *q = req->q;
	unsigned long expiry;

	if (req->cmd_flags & REQ_NO_TIMEOUT)
		return;

	/* blk-mq has its own handler, so we don't need ->rq_timed_out_fn */
	if (!q->mq_ops && !q->rq_timed_out_fn)
		return;
