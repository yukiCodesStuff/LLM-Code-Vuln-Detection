	wake_up_all(&q->mq_freeze_wq);
}

static void blk_mq_freeze_queue_start(struct request_queue *q)
{
	bool freeze;

	spin_lock_irq(q->queue_lock);
		blk_mq_run_queues(q, false);
	}
}

static void blk_mq_freeze_queue_wait(struct request_queue *q)
{
	wait_event(q->mq_freeze_wq, percpu_ref_is_zero(&q->mq_usage_counter));
	blk_mq_freeze_queue_wait(q);
}

static void blk_mq_unfreeze_queue(struct request_queue *q)
{
	bool wake;

	spin_lock_irq(q->queue_lock);
		wake_up_all(&q->mq_freeze_wq);
	}
}

bool blk_mq_can_queue(struct blk_mq_hw_ctx *hctx)
{
	return blk_mq_has_free_tags(hctx->tags);
		ctx = alloc_data.ctx;
	}
	blk_mq_put_ctx(ctx);
	if (!rq)
		return ERR_PTR(-EWOULDBLOCK);
	return rq;
}
EXPORT_SYMBOL(blk_mq_alloc_request);

}
EXPORT_SYMBOL(blk_mq_complete_request);

void blk_mq_start_request(struct request *rq)
{
	struct request_queue *q = rq->q;

}
EXPORT_SYMBOL(blk_mq_add_to_requeue_list);

void blk_mq_kick_requeue_list(struct request_queue *q)
{
	kblockd_schedule_work(&q->requeue_work);
}
EXPORT_SYMBOL(blk_mq_kick_requeue_list);

static inline bool is_flush_request(struct request *rq,
		struct blk_flush_queue *fq, unsigned int tag)
{
	return ((rq->cmd_flags & REQ_FLUSH_SEQ) &&
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
	struct blk_mq_hw_ctx *hctx;
	unsigned int i;

	queue_for_each_hw_ctx(q, hctx, i) {
		free_cpumask_var(hctx->cpumask);
		kfree(hctx);
	}
}

static int blk_mq_init_hctx(struct request_queue *q,
		struct blk_mq_tag_set *set,
	hctx->queue = q;
	hctx->queue_num = hctx_idx;
	hctx->flags = set->flags;
	hctx->cmd_size = set->cmd_size;

	blk_mq_init_cpu_notifier(&hctx->cpu_notifier,
					blk_mq_hctx_notify, hctx);
	blk_mq_register_cpu_notifier(&hctx->cpu_notifier);

	percpu_ref_exit(&q->mq_usage_counter);

	free_percpu(q->queue_ctx);
	kfree(q->queue_hw_ctx);
	kfree(q->mq_map);

	q->queue_ctx = NULL;
	q->queue_hw_ctx = NULL;
	q->mq_map = NULL;

	mutex_lock(&all_q_mutex);