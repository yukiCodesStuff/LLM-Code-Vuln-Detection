	dma_addr_t cq_dma_addr;
	u32 __iomem *q_db;
	u16 q_depth;
	s16 cq_vector;
	u16 sq_head;
	u16 sq_tail;
	u16 cq_head;
	u16 qid;
	cmd->fn = handler;
	cmd->ctx = ctx;
	cmd->aborted = 0;
	blk_mq_start_request(blk_mq_rq_from_pdu(cmd));
}

/* Special values must be less than 0x1000 */
#define CMD_CTX_BASE		((void *)POISON_POINTER_DELTA)
	if (unlikely(status)) {
		if (!(status & NVME_SC_DNR || blk_noretry_request(req))
		    && (jiffies - req->start_time) < req->timeout) {
			unsigned long flags;

			blk_mq_requeue_request(req);
			spin_lock_irqsave(req->q->queue_lock, flags);
			if (!blk_queue_stopped(req->q))
				blk_mq_kick_requeue_list(req->q);
			spin_unlock_irqrestore(req->q->queue_lock, flags);
			return;
		}
		req->errors = nvme_error_status(status);
	} else
		}
	}

	nvme_set_info(cmd, iod, req_completion);
	spin_lock_irq(&nvmeq->q_lock);
	if (req->cmd_flags & REQ_DISCARD)
		nvme_submit_discard(nvmeq, ns, req, iod);
	if (IS_ERR(req))
		return PTR_ERR(req);

	req->cmd_flags |= REQ_NO_TIMEOUT;
	cmd_info = blk_mq_rq_to_pdu(req);
	nvme_set_info(cmd_info, req, async_req_completion);

	memset(&c, 0, sizeof(c));
	struct nvme_command cmd;

	if (!nvmeq->qid || cmd_rq->aborted) {
		unsigned long flags;

		spin_lock_irqsave(&dev_list_lock, flags);
		if (work_busy(&dev->reset_work))
			goto out;
		list_del_init(&dev->node);
		dev_warn(&dev->pci_dev->dev,
			"I/O %d QID %d timeout, reset controller\n",
							req->tag, nvmeq->qid);
		dev->reset_workfn = nvme_reset_failed_dev;
		queue_work(nvme_workq, &dev->reset_work);
 out:
		spin_unlock_irqrestore(&dev_list_lock, flags);
		return;
	}

	if (!dev->abort_limit)
	void *ctx;
	nvme_completion_fn fn;
	struct nvme_cmd_info *cmd;
	struct nvme_completion cqe;

	if (!blk_mq_request_started(req))
		return;

	cmd = blk_mq_rq_to_pdu(req);

	if (cmd->ctx == CMD_CTX_CANCELLED)
		return;

	if (blk_queue_dying(req->q))
		cqe.status = cpu_to_le16((NVME_SC_ABORT_REQ | NVME_SC_DNR) << 1);
	else
		cqe.status = cpu_to_le16(NVME_SC_ABORT_REQ << 1);


	dev_warn(nvmeq->q_dmadev, "Cancelling I/O %d QID %d\n",
						req->tag, nvmeq->qid);
	ctx = cancel_cmd_info(cmd, &fn);
	fn(nvmeq, ctx, &cqe);
	struct nvme_cmd_info *cmd = blk_mq_rq_to_pdu(req);
	struct nvme_queue *nvmeq = cmd->nvmeq;

	/*
	 * The aborted req will be completed on receiving the abort req.
	 * We enable the timer again. If hit twice, it'll cause a device reset,
	 * as the device then is in a faulty state.
	 */
	int ret = BLK_EH_RESET_TIMER;

	dev_warn(nvmeq->q_dmadev, "Timeout I/O %d QID %d\n", req->tag,
							nvmeq->qid);

	spin_lock_irq(&nvmeq->q_lock);
	if (!nvmeq->dev->initialized) {
		/*
		 * Force cancelled command frees the request, which requires we
		 * return BLK_EH_NOT_HANDLED.
		 */
		nvme_cancel_queue_ios(nvmeq->hctx, req, nvmeq, reserved);
		ret = BLK_EH_NOT_HANDLED;
	} else
		nvme_abort_req(req);
	spin_unlock_irq(&nvmeq->q_lock);

	return ret;
}

static void nvme_free_queue(struct nvme_queue *nvmeq)
{
 */
static int nvme_suspend_queue(struct nvme_queue *nvmeq)
{
	int vector;

	spin_lock_irq(&nvmeq->q_lock);
	if (nvmeq->cq_vector == -1) {
		spin_unlock_irq(&nvmeq->q_lock);
		return 1;
	}
	vector = nvmeq->dev->entry[nvmeq->cq_vector].vector;
	nvmeq->dev->online_queues--;
	nvmeq->cq_vector = -1;
	spin_unlock_irq(&nvmeq->q_lock);

	irq_set_affinity_hint(vector, NULL);
	free_irq(vector, nvmeq);
		adapter_delete_sq(dev, qid);
		adapter_delete_cq(dev, qid);
	}
	if (!qid && dev->admin_q)
		blk_mq_freeze_queue_start(dev->admin_q);
	nvme_clear_queue(nvmeq);
}

static struct nvme_queue *nvme_alloc_queue(struct nvme_dev *dev, int qid,
							int depth)
{
	struct device *dmadev = &dev->pci_dev->dev;
	struct nvme_queue *nvmeq = kzalloc(sizeof(*nvmeq), GFP_KERNEL);
	if (!nvmeq)
	nvmeq->cq_phase = 1;
	nvmeq->q_db = &dev->dbs[qid * 2 * dev->db_stride];
	nvmeq->q_depth = depth;
	nvmeq->qid = qid;
	dev->queue_count++;
	dev->queues[qid] = nvmeq;

	struct nvme_dev *dev = nvmeq->dev;
	int result;

	nvmeq->cq_vector = qid - 1;
	result = adapter_alloc_cq(dev, qid, nvmeq);
	if (result < 0)
		return result;

	.timeout	= nvme_timeout,
};

static void nvme_dev_remove_admin(struct nvme_dev *dev)
{
	if (dev->admin_q && !blk_queue_dying(dev->admin_q)) {
		blk_cleanup_queue(dev->admin_q);
		blk_mq_free_tag_set(&dev->admin_tagset);
	}
}

static int nvme_alloc_admin_tags(struct nvme_dev *dev)
{
	if (!dev->admin_q) {
		dev->admin_tagset.ops = &nvme_mq_admin_ops;
			return -ENOMEM;

		dev->admin_q = blk_mq_init_queue(&dev->admin_tagset);
		if (IS_ERR(dev->admin_q)) {
			blk_mq_free_tag_set(&dev->admin_tagset);
			return -ENOMEM;
		}
		if (!blk_get_queue(dev->admin_q)) {
			nvme_dev_remove_admin(dev);
			return -ENODEV;
		}
	} else
		blk_mq_unfreeze_queue(dev->admin_q);

	return 0;
}

static int nvme_configure_admin_queue(struct nvme_dev *dev)
{
	int result;
	u32 aqa;

	nvmeq = dev->queues[0];
	if (!nvmeq) {
		nvmeq = nvme_alloc_queue(dev, 0, NVME_AQ_DEPTH);
		if (!nvmeq)
			return -ENOMEM;
	}

	if (result)
		goto free_nvmeq;

	nvmeq->cq_vector = 0;
	result = queue_request_irq(dev, nvmeq, nvmeq->irqname);
	if (result)
		goto free_nvmeq;

	return result;

 free_nvmeq:
	nvme_free_queues(dev, 0);
	return result;
}
	unsigned i;

	for (i = dev->queue_count; i <= dev->max_qid; i++)
		if (!nvme_alloc_queue(dev, i, dev->q_depth))
			break;

	for (i = dev->online_queues; i <= dev->queue_count - 1; i++)
		if (nvme_create_queue(dev->queues[i], i))
			break;
		if (!schedule_timeout(ADMIN_TIMEOUT) ||
					fatal_signal_pending(current)) {
			/*
			 * Disable the controller first since we can't trust it
			 * at this point, but leave the admin queue enabled
			 * until all queue deletion requests are flushed.
			 * FIXME: This may take a while if there are more h/w
			 * queues than admin tags.
			 */
			set_current_state(TASK_RUNNING);
			nvme_disable_ctrl(dev, readq(&dev->bar->cap));
			nvme_clear_queue(dev->queues[0]);
			flush_kthread_worker(dq->worker);
			nvme_disable_queue(dev, 0);
			return;
		}
	}
	set_current_state(TASK_RUNNING);
{
	struct nvme_queue *nvmeq = container_of(work, struct nvme_queue,
							cmdinfo.work);
	if (nvme_delete_sq(nvmeq))
		nvme_del_queue_end(nvmeq);
}

		kthread_stop(tmp);
}

static void nvme_freeze_queues(struct nvme_dev *dev)
{
	struct nvme_ns *ns;

	list_for_each_entry(ns, &dev->namespaces, list) {
		blk_mq_freeze_queue_start(ns->queue);

		spin_lock(ns->queue->queue_lock);
		queue_flag_set(QUEUE_FLAG_STOPPED, ns->queue);
		spin_unlock(ns->queue->queue_lock);

		blk_mq_cancel_requeue_work(ns->queue);
		blk_mq_stop_hw_queues(ns->queue);
	}
}

static void nvme_unfreeze_queues(struct nvme_dev *dev)
{
	struct nvme_ns *ns;

	list_for_each_entry(ns, &dev->namespaces, list) {
		queue_flag_clear_unlocked(QUEUE_FLAG_STOPPED, ns->queue);
		blk_mq_unfreeze_queue(ns->queue);
		blk_mq_start_stopped_hw_queues(ns->queue, true);
		blk_mq_kick_requeue_list(ns->queue);
	}
}

static void nvme_dev_shutdown(struct nvme_dev *dev)
{
	int i;
	u32 csts = -1;
	dev->initialized = 0;
	nvme_dev_list_remove(dev);

	if (dev->bar) {
		nvme_freeze_queues(dev);
		csts = readl(&dev->bar->csts);
	}
	if (csts & NVME_CSTS_CFS || !(csts & NVME_CSTS_RDY)) {
		for (i = dev->queue_count - 1; i >= 0; i--) {
			struct nvme_queue *nvmeq = dev->queues[i];
			nvme_suspend_queue(nvmeq);
	nvme_dev_unmap(dev);
}

static void nvme_dev_remove(struct nvme_dev *dev)
{
	struct nvme_ns *ns;

	list_for_each_entry(ns, &dev->namespaces, list) {
		if (ns->disk->flags & GENHD_FL_UP)
			del_gendisk(ns->disk);
		if (!blk_queue_dying(ns->queue)) {
			blk_mq_abort_requeue_list(ns->queue);
			blk_cleanup_queue(ns->queue);
		}
	}
}

static int nvme_setup_prp_pools(struct nvme_dev *dev)
	nvme_free_namespaces(dev);
	nvme_release_instance(dev);
	blk_mq_free_tag_set(&dev->tagset);
	blk_put_queue(dev->admin_q);
	kfree(dev->queues);
	kfree(dev->entry);
	kfree(dev);
}
	}

	nvme_init_queue(dev->queues[0], 0);
	result = nvme_alloc_admin_tags(dev);
	if (result)
		goto disable;

	result = nvme_setup_io_queues(dev);
	if (result)
		goto free_tags;

	nvme_set_irq_hints(dev);

	return result;

 free_tags:
	nvme_dev_remove_admin(dev);
 disable:
	nvme_disable_queue(dev, 0);
	nvme_dev_list_remove(dev);
 unmap:
		dev->reset_workfn = nvme_remove_disks;
		queue_work(nvme_workq, &dev->reset_work);
		spin_unlock(&dev_list_lock);
	} else {
		nvme_unfreeze_queues(dev);
		nvme_set_irq_hints(dev);
	}
	dev->initialized = 1;
	return 0;
}
	pci_set_drvdata(pdev, NULL);
	flush_work(&dev->reset_work);
	misc_deregister(&dev->miscdev);
	nvme_dev_shutdown(dev);
	nvme_dev_remove(dev);
	nvme_dev_remove_admin(dev);
	nvme_free_queues(dev, 0);
	nvme_release_prp_pools(dev);
	kref_put(&dev->kref, nvme_free_dev);
}
