{
	struct nitrox_device *ndev = cmdq->ndev;
	struct nitrox_softreq *sr;
	int req_completed = 0, err = 0, budget;
	completion_t callback;
	void *cb_arg;

	/* check all pending requests */
	budget = atomic_read(&cmdq->pending_count);

	while (req_completed < budget) {
		sr = get_first_response_entry(cmdq);
		if (!sr)
			break;

		if (atomic_read(&sr->status) != REQ_POSTED)
			break;

		/* check orh and completion bytes updates */
		if (!sr_completed(sr)) {
			/* request not completed, check for timeout */
			if (!cmd_timeout(sr->tstamp, ndev->timeout))
				break;
			dev_err_ratelimited(DEV(ndev),
					    "Request timeout, orh 0x%016llx\n",
					    READ_ONCE(*sr->resp.orh));
		}
		atomic_dec(&cmdq->pending_count);
		atomic64_inc(&ndev->stats.completed);
		/* sync with other cpus */
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
		if (!sr_completed(sr)) {
			/* request not completed, check for timeout */
			if (!cmd_timeout(sr->tstamp, ndev->timeout))
				break;
			dev_err_ratelimited(DEV(ndev),
					    "Request timeout, orh 0x%016llx\n",
					    READ_ONCE(*sr->resp.orh));
		}
		atomic_dec(&cmdq->pending_count);
		atomic64_inc(&ndev->stats.completed);
		/* sync with other cpus */
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

/**
 * pkt_slc_resp_tasklet - post processing of SE responses
 */
void pkt_slc_resp_tasklet(unsigned long data)
{
	struct nitrox_q_vector *qvec = (void *)(uintptr_t)(data);
	struct nitrox_cmdq *cmdq = qvec->cmdq;
	union nps_pkt_slc_cnts slc_cnts;

	/* read completion count */
	slc_cnts.value = readq(cmdq->compl_cnt_csr_addr);
	/* resend the interrupt if more work to do */
	slc_cnts.s.resend = 1;

	process_response_list(cmdq);

	/*
	 * clear the interrupt with resend bit enabled,
	 * MSI-X interrupt generates if Completion count > Threshold
	 */
	writeq(slc_cnts.value, cmdq->compl_cnt_csr_addr);
	/* order the writes */
	mmiowb();

	if (atomic_read(&cmdq->backlog_count))
		schedule_work(&cmdq->backlog_qflush);
}