	struct nitrox_device *ndev = cmdq->ndev;
	struct nitrox_softreq *sr;
	int req_completed = 0, err = 0, budget;
	completion_t callback;
	void *cb_arg;

	/* check all pending requests */
	budget = atomic_read(&cmdq->pending_count);

		smp_mb__after_atomic();
		/* remove from response list */
		response_list_del(sr, cmdq);
		/* ORH error code */
		err = READ_ONCE(*sr->resp.orh) & 0xff;
		callback = sr->callback;
		cb_arg = sr->cb_arg;
		softreq_destroy(sr);
		if (callback)
			callback(cb_arg, err);

		req_completed++;
	}
}