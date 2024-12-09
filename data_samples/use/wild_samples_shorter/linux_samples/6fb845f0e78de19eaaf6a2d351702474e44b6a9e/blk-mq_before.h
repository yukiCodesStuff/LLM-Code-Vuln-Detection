	struct kobject		kobj;
} ____cacheline_aligned_in_smp;

void blk_mq_freeze_queue(struct request_queue *q);
void blk_mq_free_queue(struct request_queue *q);
int blk_mq_update_nr_requests(struct request_queue *q, unsigned int nr);
void blk_mq_wake_waiters(struct request_queue *q);
bool blk_mq_dispatch_rq_list(struct request_queue *, struct list_head *, bool);