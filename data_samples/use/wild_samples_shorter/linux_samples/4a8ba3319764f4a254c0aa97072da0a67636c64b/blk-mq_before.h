void blk_mq_clone_flush_request(struct request *flush_rq,
		struct request *orig_rq);
int blk_mq_update_nr_requests(struct request_queue *q, unsigned int nr);

/*
 * CPU hotplug helpers
 */