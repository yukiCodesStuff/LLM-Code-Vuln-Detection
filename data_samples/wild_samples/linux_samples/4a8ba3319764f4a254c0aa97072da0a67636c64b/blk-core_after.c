{
	spin_lock_irq(q->queue_lock);
	if (!--q->bypass_depth)
		queue_flag_clear(QUEUE_FLAG_BYPASS, q);
	WARN_ON_ONCE(q->bypass_depth < 0);
	spin_unlock_irq(q->queue_lock);
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
{
	spinlock_t *lock = q->queue_lock;

	/* mark @q DYING, no new request or merges will be allowed afterwards */
	mutex_lock(&q->sysfs_lock);
	blk_set_queue_dying(q);
	spin_lock_irq(lock);

	/*
	 * A dying queue is permanently in bypass mode till released.  Note
	 * that, unlike blk_queue_bypass_start(), we aren't performing
	 * synchronize_rcu() after entering bypass mode to avoid the delay
	 * as some drivers create and destroy a lot of queues while
	 * probing.  This is still safe because blk_release_queue() will be
	 * called only after the queue refcnt drops to zero and nothing,
	 * RCU or not, would be traversing the queue by then.
	 */
	q->bypass_depth++;
	queue_flag_set(QUEUE_FLAG_BYPASS, q);

	queue_flag_set(QUEUE_FLAG_NOMERGES, q);
	queue_flag_set(QUEUE_FLAG_NOXMERGES, q);
	queue_flag_set(QUEUE_FLAG_DYING, q);
	spin_unlock_irq(lock);
	mutex_unlock(&q->sysfs_lock);

	/*
	 * Drain all requests queued before DYING marking. Set DEAD flag to
	 * prevent that q->request_fn() gets invoked after draining finished.
	 */
	if (q->mq_ops) {
		blk_mq_freeze_queue(q);
		spin_lock_irq(lock);
	} else {
		spin_lock_irq(lock);
		__blk_drain_queue(q, true);
	}
	queue_flag_set(QUEUE_FLAG_DEAD, q);
	spin_unlock_irq(lock);

	/* @q won't process any more request, flush async actions */
	del_timer_sync(&q->backing_dev_info.laptop_mode_wb_timer);
	blk_sync_queue(q);

	if (q->mq_ops)
		blk_mq_free_queue(q);

	spin_lock_irq(lock);
	if (q->queue_lock != &q->__queue_lock)
		q->queue_lock = &q->__queue_lock;
	spin_unlock_irq(lock);

	/* @q is and will stay empty, shutdown and put */
	blk_put_queue(q);
}