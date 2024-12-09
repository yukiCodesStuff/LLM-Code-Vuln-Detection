{
	struct request_queue *q =
		container_of(ref, struct request_queue, mq_usage_counter);

	wake_up_all(&q->mq_freeze_wq);
}

static void blk_mq_freeze_queue_start(struct request_queue *q)
{
	bool freeze;

	spin_lock_irq(q->queue_lock);
	freeze = !q->mq_freeze_depth++;
	spin_unlock_irq(q->queue_lock);

	if (freeze) {
		percpu_ref_kill(&q->mq_usage_counter);
		blk_mq_run_queues(q, false);
	}
	if (freeze) {
		percpu_ref_kill(&q->mq_usage_counter);
		blk_mq_run_queues(q, false);
	}
}

static void blk_mq_freeze_queue_wait(struct request_queue *q)
{
	wait_event(q->mq_freeze_wq, percpu_ref_is_zero(&q->mq_usage_counter));
}

/*
 * Guarantee no request is in use, so we can change any data structure of
 * the queue afterward.
 */
void blk_mq_freeze_queue(struct request_queue *q)
{
	blk_mq_freeze_queue_start(q);
	blk_mq_freeze_queue_wait(q);
}

static void blk_mq_unfreeze_queue(struct request_queue *q)
{
	bool wake;

	spin_lock_irq(q->queue_lock);
	wake = !--q->mq_freeze_depth;
	WARN_ON_ONCE(q->mq_freeze_depth < 0);
	spin_unlock_irq(q->queue_lock);
	if (wake) {
		percpu_ref_reinit(&q->mq_usage_counter);
		wake_up_all(&q->mq_freeze_wq);
	}
}

bool blk_mq_can_queue(struct blk_mq_hw_ctx *hctx)
{
	return blk_mq_has_free_tags(hctx->tags);
}
EXPORT_SYMBOL(blk_mq_can_queue);

static void blk_mq_rq_ctx_init(struct request_queue *q, struct blk_mq_ctx *ctx,
			       struct request *rq, unsigned int rw_flags)
{
	if (blk_queue_io_stat(q))
		rw_flags |= REQ_IO_STAT;

	INIT_LIST_HEAD(&rq->queuelist);
	/* csd/requeue_work/fifo_time is initialized before use */
	rq->q = q;
	rq->mq_ctx = ctx;
	rq->cmd_flags |= rw_flags;
	/* do not touch atomic flags, it needs atomic ops against the timer */
	rq->cpu = -1;
	INIT_HLIST_NODE(&rq->hash);
	RB_CLEAR_NODE(&rq->rb_node);
	rq->rq_disk = NULL;
	rq->part = NULL;
	rq->start_time = jiffies;
#ifdef CONFIG_BLK_CGROUP
	rq->rl = NULL;
	set_start_time_ns(rq);
	rq->io_start_time_ns = 0;
#endif
	rq->nr_phys_segments = 0;
#if defined(CONFIG_BLK_DEV_INTEGRITY)
	rq->nr_integrity_segments = 0;
#endif
	rq->special = NULL;
	/* tag was already set */
	rq->errors = 0;

	rq->cmd = rq->__cmd;

	rq->extra_len = 0;
	rq->sense_len = 0;
	rq->resid_len = 0;
	rq->sense = NULL;

	INIT_LIST_HEAD(&rq->timeout_list);
	rq->timeout = 0;

	rq->end_io = NULL;
	rq->end_io_data = NULL;
	rq->next_rq = NULL;

	ctx->rq_dispatched[rw_is_sync(rw_flags)]++;
}

static struct request *
__blk_mq_alloc_request(struct blk_mq_alloc_data *data, int rw)
{
	struct request *rq;
	unsigned int tag;

	tag = blk_mq_get_tag(data);
	if (tag != BLK_MQ_TAG_FAIL) {
		rq = data->hctx->tags->rqs[tag];

		if (blk_mq_tag_busy(data->hctx)) {
			rq->cmd_flags = REQ_MQ_INFLIGHT;
			atomic_inc(&data->hctx->nr_active);
		}
{
	blk_mq_freeze_queue_start(q);
	blk_mq_freeze_queue_wait(q);
}

static void blk_mq_unfreeze_queue(struct request_queue *q)
{
	bool wake;

	spin_lock_irq(q->queue_lock);
	wake = !--q->mq_freeze_depth;
	WARN_ON_ONCE(q->mq_freeze_depth < 0);
	spin_unlock_irq(q->queue_lock);
	if (wake) {
		percpu_ref_reinit(&q->mq_usage_counter);
		wake_up_all(&q->mq_freeze_wq);
	}
	if (wake) {
		percpu_ref_reinit(&q->mq_usage_counter);
		wake_up_all(&q->mq_freeze_wq);
	}
}

bool blk_mq_can_queue(struct blk_mq_hw_ctx *hctx)
{
	return blk_mq_has_free_tags(hctx->tags);
}
EXPORT_SYMBOL(blk_mq_can_queue);

static void blk_mq_rq_ctx_init(struct request_queue *q, struct blk_mq_ctx *ctx,
			       struct request *rq, unsigned int rw_flags)
{
	if (blk_queue_io_stat(q))
		rw_flags |= REQ_IO_STAT;

	INIT_LIST_HEAD(&rq->queuelist);
	/* csd/requeue_work/fifo_time is initialized before use */
	rq->q = q;
	rq->mq_ctx = ctx;
	rq->cmd_flags |= rw_flags;
	/* do not touch atomic flags, it needs atomic ops against the timer */
	rq->cpu = -1;
	INIT_HLIST_NODE(&rq->hash);
	RB_CLEAR_NODE(&rq->rb_node);
	rq->rq_disk = NULL;
	rq->part = NULL;
	rq->start_time = jiffies;
#ifdef CONFIG_BLK_CGROUP
	rq->rl = NULL;
	set_start_time_ns(rq);
	rq->io_start_time_ns = 0;
#endif
	rq->nr_phys_segments = 0;
#if defined(CONFIG_BLK_DEV_INTEGRITY)
	rq->nr_integrity_segments = 0;
#endif
	rq->special = NULL;
	/* tag was already set */
	rq->errors = 0;

	rq->cmd = rq->__cmd;

	rq->extra_len = 0;
	rq->sense_len = 0;
	rq->resid_len = 0;
	rq->sense = NULL;

	INIT_LIST_HEAD(&rq->timeout_list);
	rq->timeout = 0;

	rq->end_io = NULL;
	rq->end_io_data = NULL;
	rq->next_rq = NULL;

	ctx->rq_dispatched[rw_is_sync(rw_flags)]++;
}

static struct request *
__blk_mq_alloc_request(struct blk_mq_alloc_data *data, int rw)
{
	struct request *rq;
	unsigned int tag;

	tag = blk_mq_get_tag(data);
	if (tag != BLK_MQ_TAG_FAIL) {
		rq = data->hctx->tags->rqs[tag];

		if (blk_mq_tag_busy(data->hctx)) {
			rq->cmd_flags = REQ_MQ_INFLIGHT;
			atomic_inc(&data->hctx->nr_active);
		}
	if (!rq && (gfp & __GFP_WAIT)) {
		__blk_mq_run_hw_queue(hctx);
		blk_mq_put_ctx(ctx);

		ctx = blk_mq_get_ctx(q);
		hctx = q->mq_ops->map_queue(q, ctx->cpu);
		blk_mq_set_alloc_data(&alloc_data, q, gfp, reserved, ctx,
				hctx);
		rq =  __blk_mq_alloc_request(&alloc_data, rw);
		ctx = alloc_data.ctx;
	}
	blk_mq_put_ctx(ctx);
	if (!rq)
		return ERR_PTR(-EWOULDBLOCK);
	return rq;
}
EXPORT_SYMBOL(blk_mq_alloc_request);

static void __blk_mq_free_request(struct blk_mq_hw_ctx *hctx,
				  struct blk_mq_ctx *ctx, struct request *rq)
{
	const int tag = rq->tag;
	struct request_queue *q = rq->q;

	if (rq->cmd_flags & REQ_MQ_INFLIGHT)
		atomic_dec(&hctx->nr_active);
	rq->cmd_flags = 0;

	clear_bit(REQ_ATOM_STARTED, &rq->atomic_flags);
	blk_mq_put_tag(hctx, tag, &ctx->last_tag);
	blk_mq_queue_exit(q);
}

void blk_mq_free_hctx_request(struct blk_mq_hw_ctx *hctx, struct request *rq)
{
	struct blk_mq_ctx *ctx = rq->mq_ctx;

	ctx->rq_completed[rq_is_sync(rq)]++;
	__blk_mq_free_request(hctx, ctx, rq);

}
EXPORT_SYMBOL_GPL(blk_mq_free_hctx_request);

void blk_mq_free_request(struct request *rq)
{
	struct blk_mq_hw_ctx *hctx;
	struct request_queue *q = rq->q;

	hctx = q->mq_ops->map_queue(q, rq->mq_ctx->cpu);
	blk_mq_free_hctx_request(hctx, rq);
}
EXPORT_SYMBOL_GPL(blk_mq_free_request);

inline void __blk_mq_end_request(struct request *rq, int error)
{
	blk_account_io_done(rq);

	if (rq->end_io) {
		rq->end_io(rq, error);
	} else {
		if (unlikely(blk_bidi_rq(rq)))
			blk_mq_free_request(rq->next_rq);
		blk_mq_free_request(rq);
	}
{
	struct request_queue *q = rq->q;

	if (unlikely(blk_should_fake_timeout(q)))
		return;
	if (!blk_mark_rq_complete(rq))
		__blk_mq_complete_request(rq);
}
EXPORT_SYMBOL(blk_mq_complete_request);

void blk_mq_start_request(struct request *rq)
{
	struct request_queue *q = rq->q;

	trace_block_rq_issue(q, rq);

	rq->resid_len = blk_rq_bytes(rq);
	if (unlikely(blk_bidi_rq(rq)))
		rq->next_rq->resid_len = blk_rq_bytes(rq->next_rq);

	blk_add_timer(rq);

	/*
	 * Ensure that ->deadline is visible before set the started
	 * flag and clear the completed flag.
	 */
	smp_mb__before_atomic();

	/*
	 * Mark us as started and clear complete. Complete might have been
	 * set if requeue raced with timeout, which then marked it as
	 * complete. So be sure to clear complete again when we start
	 * the request, otherwise we'll ignore the completion event.
	 */
	if (!test_bit(REQ_ATOM_STARTED, &rq->atomic_flags))
		set_bit(REQ_ATOM_STARTED, &rq->atomic_flags);
	if (test_bit(REQ_ATOM_COMPLETE, &rq->atomic_flags))
		clear_bit(REQ_ATOM_COMPLETE, &rq->atomic_flags);

	if (q->dma_drain_size && blk_rq_bytes(rq)) {
		/*
		 * Make sure space for the drain appears.  We know we can do
		 * this because max_hw_segments has been adjusted to be one
		 * fewer than the device can handle.
		 */
		rq->nr_phys_segments++;
	}
}
	} else {
		list_add_tail(&rq->queuelist, &q->requeue_list);
	}
	spin_unlock_irqrestore(&q->requeue_lock, flags);
}
EXPORT_SYMBOL(blk_mq_add_to_requeue_list);

void blk_mq_kick_requeue_list(struct request_queue *q)
{
	kblockd_schedule_work(&q->requeue_work);
}
	switch (ret) {
	case BLK_EH_HANDLED:
		__blk_mq_complete_request(req);
		break;
	case BLK_EH_RESET_TIMER:
		blk_add_timer(req);
		blk_clear_rq_complete(req);
		break;
	case BLK_EH_NOT_HANDLED:
		break;
	default:
		printk(KERN_ERR "block: bad eh return: %d\n", ret);
		break;
	}
}
		
static void blk_mq_check_expired(struct blk_mq_hw_ctx *hctx,
		struct request *rq, void *priv, bool reserved)
{
	struct blk_mq_timeout_data *data = priv;

	if (!test_bit(REQ_ATOM_STARTED, &rq->atomic_flags))
		return;

	if (time_after_eq(jiffies, rq->deadline)) {
		if (!blk_mark_rq_complete(rq))
			blk_mq_rq_timed_out(rq, reserved);
	} else if (!data->next_set || time_after(data->next, rq->deadline)) {
		data->next = rq->deadline;
		data->next_set = 1;
	}
{
	struct blk_mq_hw_ctx *hctx;
	unsigned int i;

	queue_for_each_hw_ctx(q, hctx, i) {
		free_cpumask_var(hctx->cpumask);
		kfree(hctx);
	}
{
	int node;
	unsigned flush_start_tag = set->queue_depth;

	node = hctx->numa_node;
	if (node == NUMA_NO_NODE)
		node = hctx->numa_node = set->numa_node;

	INIT_DELAYED_WORK(&hctx->run_work, blk_mq_run_work_fn);
	INIT_DELAYED_WORK(&hctx->delay_work, blk_mq_delay_work_fn);
	spin_lock_init(&hctx->lock);
	INIT_LIST_HEAD(&hctx->dispatch);
	hctx->queue = q;
	hctx->queue_num = hctx_idx;
	hctx->flags = set->flags;
	hctx->cmd_size = set->cmd_size;

	blk_mq_init_cpu_notifier(&hctx->cpu_notifier,
					blk_mq_hctx_notify, hctx);
	blk_mq_register_cpu_notifier(&hctx->cpu_notifier);

	hctx->tags = set->tags[hctx_idx];

	/*
	 * Allocate space for all possible cpus to avoid allocation at
	 * runtime
	 */
	hctx->ctxs = kmalloc_node(nr_cpu_ids * sizeof(void *),
					GFP_KERNEL, node);
	if (!hctx->ctxs)
		goto unregister_cpu_notifier;

	if (blk_mq_alloc_bitmap(&hctx->ctx_map, node))
		goto free_ctxs;

	hctx->nr_ctx = 0;

	if (set->ops->init_hctx &&
	    set->ops->init_hctx(hctx, set->driver_data, hctx_idx))
		goto free_bitmap;

	hctx->fq = blk_alloc_flush_queue(q, hctx->numa_node, set->cmd_size);
	if (!hctx->fq)
		goto exit_hctx;

	if (set->ops->init_request &&
	    set->ops->init_request(set->driver_data,
				   hctx->fq->flush_rq, hctx_idx,
				   flush_start_tag + hctx_idx, node))
		goto free_fq;

	return 0;

 free_fq:
	kfree(hctx->fq);
 exit_hctx:
	if (set->ops->exit_hctx)
		set->ops->exit_hctx(hctx, hctx_idx);
 free_bitmap:
	blk_mq_free_bitmap(&hctx->ctx_map);
 free_ctxs:
	kfree(hctx->ctxs);
 unregister_cpu_notifier:
	blk_mq_unregister_cpu_notifier(&hctx->cpu_notifier);

	return -1;
}

static int blk_mq_init_hw_queues(struct request_queue *q,
		struct blk_mq_tag_set *set)
{
	struct blk_mq_hw_ctx *hctx;
	unsigned int i;

	/*
	 * Initialize hardware queues
	 */
	queue_for_each_hw_ctx(q, hctx, i) {
		if (blk_mq_init_hctx(q, set, hctx, i))
			break;
	}
{
	struct blk_mq_tag_set	*set = q->tag_set;

	blk_mq_del_queue_tag_set(q);

	blk_mq_exit_hw_queues(q, set, set->nr_hw_queues);
	blk_mq_free_hw_queues(q, set);

	percpu_ref_exit(&q->mq_usage_counter);

	free_percpu(q->queue_ctx);
	kfree(q->queue_hw_ctx);
	kfree(q->mq_map);

	q->queue_ctx = NULL;
	q->queue_hw_ctx = NULL;
	q->mq_map = NULL;

	mutex_lock(&all_q_mutex);
	list_del_init(&q->all_q_node);
	mutex_unlock(&all_q_mutex);
}

/* Basically redo blk_mq_init_queue with queue frozen */
static void blk_mq_queue_reinit(struct request_queue *q)
{
	WARN_ON_ONCE(!q->mq_freeze_depth);

	blk_mq_sysfs_unregister(q);

	blk_mq_update_queue_map(q->mq_map, q->nr_hw_queues);

	/*
	 * redo blk_mq_init_cpu_queues and blk_mq_init_hw_queues. FIXME: maybe
	 * we should change hctx numa_node according to new topology (this
	 * involves free and re-allocate memory, worthy doing?)
	 */

	blk_mq_map_swqueue(q);

	blk_mq_sysfs_register(q);
}

static int blk_mq_queue_reinit_notify(struct notifier_block *nb,
				      unsigned long action, void *hcpu)
{
	struct request_queue *q;

	/*
	 * Before new mappings are established, hotadded cpu might already
	 * start handling requests. This doesn't break anything as we map
	 * offline CPUs to first hardware queue. We will re-init the queue
	 * below to get optimal settings.
	 */
	if (action != CPU_DEAD && action != CPU_DEAD_FROZEN &&
	    action != CPU_ONLINE && action != CPU_ONLINE_FROZEN)
		return NOTIFY_OK;

	mutex_lock(&all_q_mutex);

	/*
	 * We need to freeze and reinit all existing queues.  Freezing
	 * involves synchronous wait for an RCU grace period and doing it
	 * one by one may take a long time.  Start freezing all queues in
	 * one swoop and then wait for the completions so that freezing can
	 * take place in parallel.
	 */
	list_for_each_entry(q, &all_q_list, all_q_node)
		blk_mq_freeze_queue_start(q);
	list_for_each_entry(q, &all_q_list, all_q_node)
		blk_mq_freeze_queue_wait(q);

	list_for_each_entry(q, &all_q_list, all_q_node)
		blk_mq_queue_reinit(q);

	list_for_each_entry(q, &all_q_list, all_q_node)
		blk_mq_unfreeze_queue(q);

	mutex_unlock(&all_q_mutex);
	return NOTIFY_OK;
}

static int __blk_mq_alloc_rq_maps(struct blk_mq_tag_set *set)
{
	int i;

	for (i = 0; i < set->nr_hw_queues; i++) {
		set->tags[i] = blk_mq_init_rq_map(set, i);
		if (!set->tags[i])
			goto out_unwind;
	}