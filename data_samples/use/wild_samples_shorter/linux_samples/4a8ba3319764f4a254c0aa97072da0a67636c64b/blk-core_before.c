}
EXPORT_SYMBOL_GPL(blk_queue_bypass_end);

/**
 * blk_cleanup_queue - shutdown a request queue
 * @q: request queue to shutdown
 *

	/* mark @q DYING, no new request or merges will be allowed afterwards */
	mutex_lock(&q->sysfs_lock);
	queue_flag_set_unlocked(QUEUE_FLAG_DYING, q);
	spin_lock_irq(lock);

	/*
	 * A dying queue is permanently in bypass mode till released.  Note