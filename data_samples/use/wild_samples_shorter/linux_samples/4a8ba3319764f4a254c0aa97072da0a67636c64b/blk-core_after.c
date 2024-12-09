}
EXPORT_SYMBOL_GPL(blk_queue_bypass_end);

void blk_set_queue_dying(struct request_queue *q)
{
	queue_flag_set_unlocked(QUEUE_FLAG_DYING, q);

	if (q->mq_ops)
		blk_mq_wake_waiters(q);
	else {
		struct request_list *rl;

		blk_queue_for_each_rl(rl, q) {
			if (rl->rq_pool) {
				wake_up(&rl->wait[BLK_RW_SYNC]);
				wake_up(&rl->wait[BLK_RW_ASYNC]);
			}
		}
	}
}
EXPORT_SYMBOL_GPL(blk_set_queue_dying);

/**
 * blk_cleanup_queue - shutdown a request queue
 * @q: request queue to shutdown
 *

	/* mark @q DYING, no new request or merges will be allowed afterwards */
	mutex_lock(&q->sysfs_lock);
	blk_set_queue_dying(q);
	spin_lock_irq(lock);

	/*
	 * A dying queue is permanently in bypass mode till released.  Note