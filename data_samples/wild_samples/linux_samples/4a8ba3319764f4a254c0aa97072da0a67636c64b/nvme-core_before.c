struct nvme_queue {
	struct llist_node node;
	struct device *q_dmadev;
	struct nvme_dev *dev;
	char irqname[24];	/* nvme4294967295-65535\0 */
	spinlock_t q_lock;
	struct nvme_command *sq_cmds;
	volatile struct nvme_completion *cqes;
	dma_addr_t sq_dma_addr;
	dma_addr_t cq_dma_addr;
	u32 __iomem *q_db;
	u16 q_depth;
	u16 cq_vector;
	u16 sq_head;
	u16 sq_tail;
	u16 cq_head;
	u16 qid;
	u8 cq_phase;
	u8 cqe_seen;
	struct async_cmd_info cmdinfo;
	struct blk_mq_hw_ctx *hctx;
};

/*
 * Check we didin't inadvertently grow the command struct
 */
static inline void _nvme_check_size(void)
{
	BUILD_BUG_ON(sizeof(struct nvme_rw_command) != 64);
	BUILD_BUG_ON(sizeof(struct nvme_create_cq) != 64);
	BUILD_BUG_ON(sizeof(struct nvme_create_sq) != 64);
	BUILD_BUG_ON(sizeof(struct nvme_delete_queue) != 64);
	BUILD_BUG_ON(sizeof(struct nvme_features) != 64);
	BUILD_BUG_ON(sizeof(struct nvme_format_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct nvme_abort_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct nvme_command) != 64);
	BUILD_BUG_ON(sizeof(struct nvme_id_ctrl) != 4096);
	BUILD_BUG_ON(sizeof(struct nvme_id_ns) != 4096);
	BUILD_BUG_ON(sizeof(struct nvme_lba_range_type) != 64);
	BUILD_BUG_ON(sizeof(struct nvme_smart_log) != 512);
}
{
	cmd->fn = handler;
	cmd->ctx = ctx;
	cmd->aborted = 0;
}

/* Special values must be less than 0x1000 */
#define CMD_CTX_BASE		((void *)POISON_POINTER_DELTA)
#define CMD_CTX_CANCELLED	(0x30C + CMD_CTX_BASE)
#define CMD_CTX_COMPLETED	(0x310 + CMD_CTX_BASE)
#define CMD_CTX_INVALID		(0x314 + CMD_CTX_BASE)

static void special_completion(struct nvme_queue *nvmeq, void *ctx,
						struct nvme_completion *cqe)
{
	if (ctx == CMD_CTX_CANCELLED)
		return;
	if (ctx == CMD_CTX_COMPLETED) {
		dev_warn(nvmeq->q_dmadev,
				"completed id %d twice on queue %d\n",
				cqe->command_id, le16_to_cpup(&cqe->sq_id));
		return;
	}
	if (unlikely(status)) {
		if (!(status & NVME_SC_DNR || blk_noretry_request(req))
		    && (jiffies - req->start_time) < req->timeout) {
			blk_mq_requeue_request(req);
			blk_mq_kick_requeue_list(req->q);
			return;
		}
                    nvme_setup_prps(nvmeq->dev, iod, blk_rq_bytes(req), GFP_ATOMIC)) {
			dma_unmap_sg(&nvmeq->dev->pci_dev->dev, iod->sg,
					iod->nents, dma_dir);
			goto retry_cmd;
		}
	}

	blk_mq_start_request(req);

	nvme_set_info(cmd, iod, req_completion);
	spin_lock_irq(&nvmeq->q_lock);
	if (req->cmd_flags & REQ_DISCARD)
		nvme_submit_discard(nvmeq, ns, req, iod);
	else if (req->cmd_flags & REQ_FLUSH)
		nvme_submit_flush(nvmeq, ns, req->tag);
	else
		nvme_submit_iod(nvmeq, iod, ns);

	nvme_process_cq(nvmeq);
	spin_unlock_irq(&nvmeq->q_lock);
	return BLK_MQ_RQ_QUEUE_OK;

 error_cmd:
	nvme_free_iod(nvmeq->dev, iod);
	return BLK_MQ_RQ_QUEUE_ERROR;
 retry_cmd:
	nvme_free_iod(nvmeq->dev, iod);
	return BLK_MQ_RQ_QUEUE_BUSY;
}

static int nvme_process_cq(struct nvme_queue *nvmeq)
{
	u16 head, phase;

	head = nvmeq->cq_head;
	phase = nvmeq->cq_phase;

	for (;;) {
		void *ctx;
		nvme_completion_fn fn;
		struct nvme_completion cqe = nvmeq->cqes[head];
		if ((le16_to_cpu(cqe.status) & 1) != phase)
			break;
		nvmeq->sq_head = le16_to_cpu(cqe.sq_head);
		if (++head == nvmeq->q_depth) {
			head = 0;
			phase = !phase;
		}
{
	struct nvme_queue *nvmeq = dev->queues[0];
	struct nvme_command c;
	struct nvme_cmd_info *cmd_info;
	struct request *req;

	req = blk_mq_alloc_request(dev->admin_q, WRITE, GFP_ATOMIC, false);
	if (IS_ERR(req))
		return PTR_ERR(req);

	cmd_info = blk_mq_rq_to_pdu(req);
	nvme_set_info(cmd_info, req, async_req_completion);

	memset(&c, 0, sizeof(c));
	c.common.opcode = nvme_admin_async_event;
	c.common.command_id = req->tag;

	return __nvme_submit_cmd(nvmeq, &c);
}

static int nvme_submit_admin_async_cmd(struct nvme_dev *dev,
			struct nvme_command *cmd,
			struct async_cmd_info *cmdinfo, unsigned timeout)
{
	struct nvme_queue *nvmeq = dev->queues[0];
	struct request *req;
	struct nvme_cmd_info *cmd_rq;

	req = blk_mq_alloc_request(dev->admin_q, WRITE, GFP_KERNEL, false);
	if (IS_ERR(req))
		return PTR_ERR(req);

	req->timeout = timeout;
	cmd_rq = blk_mq_rq_to_pdu(req);
	cmdinfo->req = req;
	nvme_set_info(cmd_rq, cmdinfo, async_completion);
	cmdinfo->status = -EINTR;

	cmd->common.command_id = req->tag;

	return nvme_submit_cmd(nvmeq, cmd);
}

static int __nvme_submit_admin_cmd(struct nvme_dev *dev, struct nvme_command *cmd,
						u32 *result, unsigned timeout)
{
	int res;
	struct request *req;

	req = blk_mq_alloc_request(dev->admin_q, WRITE, GFP_KERNEL, false);
	if (IS_ERR(req))
		return PTR_ERR(req);
	res = nvme_submit_sync_cmd(req, cmd, result, timeout);
	blk_mq_free_request(req);
	return res;
}

int nvme_submit_admin_cmd(struct nvme_dev *dev, struct nvme_command *cmd,
								u32 *result)
{
	return __nvme_submit_admin_cmd(dev, cmd, result, ADMIN_TIMEOUT);
}

int nvme_submit_io_cmd(struct nvme_dev *dev, struct nvme_ns *ns,
					struct nvme_command *cmd, u32 *result)
{
	int res;
	struct request *req;

	req = blk_mq_alloc_request(ns->queue, WRITE, (GFP_KERNEL|__GFP_WAIT),
									false);
	if (IS_ERR(req))
		return PTR_ERR(req);
	res = nvme_submit_sync_cmd(req, cmd, result, NVME_IO_TIMEOUT);
	blk_mq_free_request(req);
	return res;
}

static int adapter_delete_queue(struct nvme_dev *dev, u8 opcode, u16 id)
{
	struct nvme_command c;

	memset(&c, 0, sizeof(c));
	c.delete_queue.opcode = opcode;
	c.delete_queue.qid = cpu_to_le16(id);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

static int adapter_alloc_cq(struct nvme_dev *dev, u16 qid,
						struct nvme_queue *nvmeq)
{
	struct nvme_command c;
	int flags = NVME_QUEUE_PHYS_CONTIG | NVME_CQ_IRQ_ENABLED;

	memset(&c, 0, sizeof(c));
	c.create_cq.opcode = nvme_admin_create_cq;
	c.create_cq.prp1 = cpu_to_le64(nvmeq->cq_dma_addr);
	c.create_cq.cqid = cpu_to_le16(qid);
	c.create_cq.qsize = cpu_to_le16(nvmeq->q_depth - 1);
	c.create_cq.cq_flags = cpu_to_le16(flags);
	c.create_cq.irq_vector = cpu_to_le16(nvmeq->cq_vector);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

static int adapter_alloc_sq(struct nvme_dev *dev, u16 qid,
						struct nvme_queue *nvmeq)
{
	struct nvme_command c;
	int flags = NVME_QUEUE_PHYS_CONTIG | NVME_SQ_PRIO_MEDIUM;

	memset(&c, 0, sizeof(c));
	c.create_sq.opcode = nvme_admin_create_sq;
	c.create_sq.prp1 = cpu_to_le64(nvmeq->sq_dma_addr);
	c.create_sq.sqid = cpu_to_le16(qid);
	c.create_sq.qsize = cpu_to_le16(nvmeq->q_depth - 1);
	c.create_sq.sq_flags = cpu_to_le16(flags);
	c.create_sq.cqid = cpu_to_le16(qid);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

static int adapter_delete_cq(struct nvme_dev *dev, u16 cqid)
{
	return adapter_delete_queue(dev, nvme_admin_delete_cq, cqid);
}

static int adapter_delete_sq(struct nvme_dev *dev, u16 sqid)
{
	return adapter_delete_queue(dev, nvme_admin_delete_sq, sqid);
}

int nvme_identify(struct nvme_dev *dev, unsigned nsid, unsigned cns,
							dma_addr_t dma_addr)
{
	struct nvme_command c;

	memset(&c, 0, sizeof(c));
	c.identify.opcode = nvme_admin_identify;
	c.identify.nsid = cpu_to_le32(nsid);
	c.identify.prp1 = cpu_to_le64(dma_addr);
	c.identify.cns = cpu_to_le32(cns);

	return nvme_submit_admin_cmd(dev, &c, NULL);
}

int nvme_get_features(struct nvme_dev *dev, unsigned fid, unsigned nsid,
					dma_addr_t dma_addr, u32 *result)
{
	struct nvme_command c;

	memset(&c, 0, sizeof(c));
	c.features.opcode = nvme_admin_get_features;
	c.features.nsid = cpu_to_le32(nsid);
	c.features.prp1 = cpu_to_le64(dma_addr);
	c.features.fid = cpu_to_le32(fid);

	return nvme_submit_admin_cmd(dev, &c, result);
}

int nvme_set_features(struct nvme_dev *dev, unsigned fid, unsigned dword11,
					dma_addr_t dma_addr, u32 *result)
{
	struct nvme_command c;

	memset(&c, 0, sizeof(c));
	c.features.opcode = nvme_admin_set_features;
	c.features.prp1 = cpu_to_le64(dma_addr);
	c.features.fid = cpu_to_le32(fid);
	c.features.dword11 = cpu_to_le32(dword11);

	return nvme_submit_admin_cmd(dev, &c, result);
}

/**
 * nvme_abort_req - Attempt aborting a request
 *
 * Schedule controller reset if the command was already aborted once before and
 * still hasn't been returned to the driver, or if this is the admin queue.
 */
static void nvme_abort_req(struct request *req)
{
	struct nvme_cmd_info *cmd_rq = blk_mq_rq_to_pdu(req);
	struct nvme_queue *nvmeq = cmd_rq->nvmeq;
	struct nvme_dev *dev = nvmeq->dev;
	struct request *abort_req;
	struct nvme_cmd_info *abort_cmd;
	struct nvme_command cmd;

	if (!nvmeq->qid || cmd_rq->aborted) {
		if (work_busy(&dev->reset_work))
			return;
		list_del_init(&dev->node);
		dev_warn(&dev->pci_dev->dev,
			"I/O %d QID %d timeout, reset controller\n",
							req->tag, nvmeq->qid);
		dev->reset_workfn = nvme_reset_failed_dev;
		queue_work(nvme_workq, &dev->reset_work);
		return;
	}
{
	struct nvme_cmd_info *cmd_rq = blk_mq_rq_to_pdu(req);
	struct nvme_queue *nvmeq = cmd_rq->nvmeq;
	struct nvme_dev *dev = nvmeq->dev;
	struct request *abort_req;
	struct nvme_cmd_info *abort_cmd;
	struct nvme_command cmd;

	if (!nvmeq->qid || cmd_rq->aborted) {
		if (work_busy(&dev->reset_work))
			return;
		list_del_init(&dev->node);
		dev_warn(&dev->pci_dev->dev,
			"I/O %d QID %d timeout, reset controller\n",
							req->tag, nvmeq->qid);
		dev->reset_workfn = nvme_reset_failed_dev;
		queue_work(nvme_workq, &dev->reset_work);
		return;
	}
{
	struct nvme_queue *nvmeq = data;
	void *ctx;
	nvme_completion_fn fn;
	struct nvme_cmd_info *cmd;
	static struct nvme_completion cqe = {
		.status = cpu_to_le16(NVME_SC_ABORT_REQ << 1),
	};

	cmd = blk_mq_rq_to_pdu(req);

	if (cmd->ctx == CMD_CTX_CANCELLED)
		return;

	dev_warn(nvmeq->q_dmadev, "Cancelling I/O %d QID %d\n",
						req->tag, nvmeq->qid);
	ctx = cancel_cmd_info(cmd, &fn);
	fn(nvmeq, ctx, &cqe);
}
{
	struct nvme_cmd_info *cmd = blk_mq_rq_to_pdu(req);
	struct nvme_queue *nvmeq = cmd->nvmeq;

	dev_warn(nvmeq->q_dmadev, "Timeout I/O %d QID %d\n", req->tag,
							nvmeq->qid);
	if (nvmeq->dev->initialized)
		nvme_abort_req(req);

	/*
	 * The aborted req will be completed on receiving the abort req.
	 * We enable the timer again. If hit twice, it'll cause a device reset,
	 * as the device then is in a faulty state.
	 */
	return BLK_EH_RESET_TIMER;
}

static void nvme_free_queue(struct nvme_queue *nvmeq)
{
	dma_free_coherent(nvmeq->q_dmadev, CQ_SIZE(nvmeq->q_depth),
				(void *)nvmeq->cqes, nvmeq->cq_dma_addr);
	dma_free_coherent(nvmeq->q_dmadev, SQ_SIZE(nvmeq->q_depth),
					nvmeq->sq_cmds, nvmeq->sq_dma_addr);
	kfree(nvmeq);
}

static void nvme_free_queues(struct nvme_dev *dev, int lowest)
{
	LLIST_HEAD(q_list);
	struct nvme_queue *nvmeq, *next;
	struct llist_node *entry;
	int i;

	for (i = dev->queue_count - 1; i >= lowest; i--) {
		struct nvme_queue *nvmeq = dev->queues[i];
		llist_add(&nvmeq->node, &q_list);
		dev->queue_count--;
		dev->queues[i] = NULL;
	}
	for (i = dev->queue_count - 1; i >= lowest; i--) {
		struct nvme_queue *nvmeq = dev->queues[i];
		llist_add(&nvmeq->node, &q_list);
		dev->queue_count--;
		dev->queues[i] = NULL;
	}
	synchronize_rcu();
	entry = llist_del_all(&q_list);
	llist_for_each_entry_safe(nvmeq, next, entry, node)
		nvme_free_queue(nvmeq);
}

/**
 * nvme_suspend_queue - put queue into suspended state
 * @nvmeq - queue to suspend
 */
static int nvme_suspend_queue(struct nvme_queue *nvmeq)
{
	int vector = nvmeq->dev->entry[nvmeq->cq_vector].vector;

	spin_lock_irq(&nvmeq->q_lock);
	nvmeq->dev->online_queues--;
	spin_unlock_irq(&nvmeq->q_lock);

	irq_set_affinity_hint(vector, NULL);
	free_irq(vector, nvmeq);

	return 0;
}
	if (qid && readl(&dev->bar->csts) != -1) {
		adapter_delete_sq(dev, qid);
		adapter_delete_cq(dev, qid);
	}
	nvme_clear_queue(nvmeq);
}

static struct nvme_queue *nvme_alloc_queue(struct nvme_dev *dev, int qid,
							int depth, int vector)
{
	struct device *dmadev = &dev->pci_dev->dev;
	struct nvme_queue *nvmeq = kzalloc(sizeof(*nvmeq), GFP_KERNEL);
	if (!nvmeq)
		return NULL;

	nvmeq->cqes = dma_zalloc_coherent(dmadev, CQ_SIZE(depth),
					  &nvmeq->cq_dma_addr, GFP_KERNEL);
	if (!nvmeq->cqes)
		goto free_nvmeq;

	nvmeq->sq_cmds = dma_alloc_coherent(dmadev, SQ_SIZE(depth),
					&nvmeq->sq_dma_addr, GFP_KERNEL);
	if (!nvmeq->sq_cmds)
		goto free_cqdma;

	nvmeq->q_dmadev = dmadev;
	nvmeq->dev = dev;
	snprintf(nvmeq->irqname, sizeof(nvmeq->irqname), "nvme%dq%d",
			dev->instance, qid);
	spin_lock_init(&nvmeq->q_lock);
	nvmeq->cq_head = 0;
	nvmeq->cq_phase = 1;
	nvmeq->q_db = &dev->dbs[qid * 2 * dev->db_stride];
	nvmeq->q_depth = depth;
	nvmeq->cq_vector = vector;
	nvmeq->qid = qid;
	dev->queue_count++;
	dev->queues[qid] = nvmeq;

	return nvmeq;

 free_cqdma:
	dma_free_coherent(dmadev, CQ_SIZE(depth), (void *)nvmeq->cqes,
							nvmeq->cq_dma_addr);
 free_nvmeq:
	kfree(nvmeq);
	return NULL;
}

static int queue_request_irq(struct nvme_dev *dev, struct nvme_queue *nvmeq,
							const char *name)
{
	if (use_threaded_interrupts)
		return request_threaded_irq(dev->entry[nvmeq->cq_vector].vector,
					nvme_irq_check, nvme_irq, IRQF_SHARED,
					name, nvmeq);
	return request_irq(dev->entry[nvmeq->cq_vector].vector, nvme_irq,
				IRQF_SHARED, name, nvmeq);
}

static void nvme_init_queue(struct nvme_queue *nvmeq, u16 qid)
{
	struct nvme_dev *dev = nvmeq->dev;

	spin_lock_irq(&nvmeq->q_lock);
	nvmeq->sq_tail = 0;
	nvmeq->cq_head = 0;
	nvmeq->cq_phase = 1;
	nvmeq->q_db = &dev->dbs[qid * 2 * dev->db_stride];
	memset((void *)nvmeq->cqes, 0, CQ_SIZE(nvmeq->q_depth));
	dev->online_queues++;
	spin_unlock_irq(&nvmeq->q_lock);
}

static int nvme_create_queue(struct nvme_queue *nvmeq, int qid)
{
	struct nvme_dev *dev = nvmeq->dev;
	int result;

	result = adapter_alloc_cq(dev, qid, nvmeq);
	if (result < 0)
		return result;

	result = adapter_alloc_sq(dev, qid, nvmeq);
	if (result < 0)
		goto release_cq;

	result = queue_request_irq(dev, nvmeq, nvmeq->irqname);
	if (result < 0)
		goto release_sq;

	nvme_init_queue(nvmeq, qid);
	return result;

 release_sq:
	adapter_delete_sq(dev, qid);
 release_cq:
	adapter_delete_cq(dev, qid);
	return result;
}

static int nvme_wait_ready(struct nvme_dev *dev, u64 cap, bool enabled)
{
	unsigned long timeout;
	u32 bit = enabled ? NVME_CSTS_RDY : 0;

	timeout = ((NVME_CAP_TIMEOUT(cap) + 1) * HZ / 2) + jiffies;

	while ((readl(&dev->bar->csts) & NVME_CSTS_RDY) != bit) {
		msleep(100);
		if (fatal_signal_pending(current))
			return -EINTR;
		if (time_after(jiffies, timeout)) {
			dev_err(&dev->pci_dev->dev,
				"Device not ready; aborting %s\n", enabled ?
						"initialisation" : "reset");
			return -ENODEV;
		}
{
	struct device *dmadev = &dev->pci_dev->dev;
	struct nvme_queue *nvmeq = kzalloc(sizeof(*nvmeq), GFP_KERNEL);
	if (!nvmeq)
		return NULL;

	nvmeq->cqes = dma_zalloc_coherent(dmadev, CQ_SIZE(depth),
					  &nvmeq->cq_dma_addr, GFP_KERNEL);
	if (!nvmeq->cqes)
		goto free_nvmeq;

	nvmeq->sq_cmds = dma_alloc_coherent(dmadev, SQ_SIZE(depth),
					&nvmeq->sq_dma_addr, GFP_KERNEL);
	if (!nvmeq->sq_cmds)
		goto free_cqdma;

	nvmeq->q_dmadev = dmadev;
	nvmeq->dev = dev;
	snprintf(nvmeq->irqname, sizeof(nvmeq->irqname), "nvme%dq%d",
			dev->instance, qid);
	spin_lock_init(&nvmeq->q_lock);
	nvmeq->cq_head = 0;
	nvmeq->cq_phase = 1;
	nvmeq->q_db = &dev->dbs[qid * 2 * dev->db_stride];
	nvmeq->q_depth = depth;
	nvmeq->cq_vector = vector;
	nvmeq->qid = qid;
	dev->queue_count++;
	dev->queues[qid] = nvmeq;

	return nvmeq;

 free_cqdma:
	dma_free_coherent(dmadev, CQ_SIZE(depth), (void *)nvmeq->cqes,
							nvmeq->cq_dma_addr);
 free_nvmeq:
	kfree(nvmeq);
	return NULL;
}

static int queue_request_irq(struct nvme_dev *dev, struct nvme_queue *nvmeq,
							const char *name)
{
	if (use_threaded_interrupts)
		return request_threaded_irq(dev->entry[nvmeq->cq_vector].vector,
					nvme_irq_check, nvme_irq, IRQF_SHARED,
					name, nvmeq);
	return request_irq(dev->entry[nvmeq->cq_vector].vector, nvme_irq,
				IRQF_SHARED, name, nvmeq);
}

static void nvme_init_queue(struct nvme_queue *nvmeq, u16 qid)
{
	struct nvme_dev *dev = nvmeq->dev;

	spin_lock_irq(&nvmeq->q_lock);
	nvmeq->sq_tail = 0;
	nvmeq->cq_head = 0;
	nvmeq->cq_phase = 1;
	nvmeq->q_db = &dev->dbs[qid * 2 * dev->db_stride];
	memset((void *)nvmeq->cqes, 0, CQ_SIZE(nvmeq->q_depth));
	dev->online_queues++;
	spin_unlock_irq(&nvmeq->q_lock);
}

static int nvme_create_queue(struct nvme_queue *nvmeq, int qid)
{
	struct nvme_dev *dev = nvmeq->dev;
	int result;

	result = adapter_alloc_cq(dev, qid, nvmeq);
	if (result < 0)
		return result;

	result = adapter_alloc_sq(dev, qid, nvmeq);
	if (result < 0)
		goto release_cq;

	result = queue_request_irq(dev, nvmeq, nvmeq->irqname);
	if (result < 0)
		goto release_sq;

	nvme_init_queue(nvmeq, qid);
	return result;

 release_sq:
	adapter_delete_sq(dev, qid);
 release_cq:
	adapter_delete_cq(dev, qid);
	return result;
}

static int nvme_wait_ready(struct nvme_dev *dev, u64 cap, bool enabled)
{
	unsigned long timeout;
	u32 bit = enabled ? NVME_CSTS_RDY : 0;

	timeout = ((NVME_CAP_TIMEOUT(cap) + 1) * HZ / 2) + jiffies;

	while ((readl(&dev->bar->csts) & NVME_CSTS_RDY) != bit) {
		msleep(100);
		if (fatal_signal_pending(current))
			return -EINTR;
		if (time_after(jiffies, timeout)) {
			dev_err(&dev->pci_dev->dev,
				"Device not ready; aborting %s\n", enabled ?
						"initialisation" : "reset");
			return -ENODEV;
		}
	}
{
	struct nvme_dev *dev = nvmeq->dev;
	int result;

	result = adapter_alloc_cq(dev, qid, nvmeq);
	if (result < 0)
		return result;

	result = adapter_alloc_sq(dev, qid, nvmeq);
	if (result < 0)
		goto release_cq;

	result = queue_request_irq(dev, nvmeq, nvmeq->irqname);
	if (result < 0)
		goto release_sq;

	nvme_init_queue(nvmeq, qid);
	return result;

 release_sq:
	adapter_delete_sq(dev, qid);
 release_cq:
	adapter_delete_cq(dev, qid);
	return result;
}

static int nvme_wait_ready(struct nvme_dev *dev, u64 cap, bool enabled)
{
	unsigned long timeout;
	u32 bit = enabled ? NVME_CSTS_RDY : 0;

	timeout = ((NVME_CAP_TIMEOUT(cap) + 1) * HZ / 2) + jiffies;

	while ((readl(&dev->bar->csts) & NVME_CSTS_RDY) != bit) {
		msleep(100);
		if (fatal_signal_pending(current))
			return -EINTR;
		if (time_after(jiffies, timeout)) {
			dev_err(&dev->pci_dev->dev,
				"Device not ready; aborting %s\n", enabled ?
						"initialisation" : "reset");
			return -ENODEV;
		}
	}
static struct blk_mq_ops nvme_mq_ops = {
	.queue_rq	= nvme_queue_rq,
	.map_queue	= blk_mq_map_queue,
	.init_hctx	= nvme_init_hctx,
	.exit_hctx	= nvme_exit_hctx,
	.init_request	= nvme_init_request,
	.timeout	= nvme_timeout,
};

static int nvme_alloc_admin_tags(struct nvme_dev *dev)
{
	if (!dev->admin_q) {
		dev->admin_tagset.ops = &nvme_mq_admin_ops;
		dev->admin_tagset.nr_hw_queues = 1;
		dev->admin_tagset.queue_depth = NVME_AQ_DEPTH - 1;
		dev->admin_tagset.timeout = ADMIN_TIMEOUT;
		dev->admin_tagset.numa_node = dev_to_node(&dev->pci_dev->dev);
		dev->admin_tagset.cmd_size = sizeof(struct nvme_cmd_info);
		dev->admin_tagset.driver_data = dev;

		if (blk_mq_alloc_tag_set(&dev->admin_tagset))
			return -ENOMEM;

		dev->admin_q = blk_mq_init_queue(&dev->admin_tagset);
		if (!dev->admin_q) {
			blk_mq_free_tag_set(&dev->admin_tagset);
			return -ENOMEM;
		}
	}

	return 0;
}
	if (!dev->admin_q) {
		dev->admin_tagset.ops = &nvme_mq_admin_ops;
		dev->admin_tagset.nr_hw_queues = 1;
		dev->admin_tagset.queue_depth = NVME_AQ_DEPTH - 1;
		dev->admin_tagset.timeout = ADMIN_TIMEOUT;
		dev->admin_tagset.numa_node = dev_to_node(&dev->pci_dev->dev);
		dev->admin_tagset.cmd_size = sizeof(struct nvme_cmd_info);
		dev->admin_tagset.driver_data = dev;

		if (blk_mq_alloc_tag_set(&dev->admin_tagset))
			return -ENOMEM;

		dev->admin_q = blk_mq_init_queue(&dev->admin_tagset);
		if (!dev->admin_q) {
			blk_mq_free_tag_set(&dev->admin_tagset);
			return -ENOMEM;
		}
	if (page_shift > dev_page_max) {
		dev_info(&dev->pci_dev->dev,
				"Device maximum page size (%u) smaller than "
				"host (%u); enabling work-around\n",
				1 << dev_page_max, 1 << page_shift);
		page_shift = dev_page_max;
	}

	result = nvme_disable_ctrl(dev, cap);
	if (result < 0)
		return result;

	nvmeq = dev->queues[0];
	if (!nvmeq) {
		nvmeq = nvme_alloc_queue(dev, 0, NVME_AQ_DEPTH, 0);
		if (!nvmeq)
			return -ENOMEM;
	}
	if (!nvmeq) {
		nvmeq = nvme_alloc_queue(dev, 0, NVME_AQ_DEPTH, 0);
		if (!nvmeq)
			return -ENOMEM;
	}

	aqa = nvmeq->q_depth - 1;
	aqa |= aqa << 16;

	dev->page_size = 1 << page_shift;

	dev->ctrl_config = NVME_CC_CSS_NVM;
	dev->ctrl_config |= (page_shift - 12) << NVME_CC_MPS_SHIFT;
	dev->ctrl_config |= NVME_CC_ARB_RR | NVME_CC_SHN_NONE;
	dev->ctrl_config |= NVME_CC_IOSQES | NVME_CC_IOCQES;

	writel(aqa, &dev->bar->aqa);
	writeq(nvmeq->sq_dma_addr, &dev->bar->asq);
	writeq(nvmeq->cq_dma_addr, &dev->bar->acq);

	result = nvme_enable_ctrl(dev, cap);
	if (result)
		goto free_nvmeq;

	result = nvme_alloc_admin_tags(dev);
	if (result)
		goto free_nvmeq;

	result = queue_request_irq(dev, nvmeq, nvmeq->irqname);
	if (result)
		goto free_tags;

	return result;

 free_tags:
	nvme_free_admin_tags(dev);
 free_nvmeq:
	nvme_free_queues(dev, 0);
	return result;
}

struct nvme_iod *nvme_map_user_pages(struct nvme_dev *dev, int write,
				unsigned long addr, unsigned length)
{
	int i, err, count, nents, offset;
	struct scatterlist *sg;
	struct page **pages;
	struct nvme_iod *iod;

	if (addr & 3)
		return ERR_PTR(-EINVAL);
	if (!length || length > INT_MAX - PAGE_SIZE)
		return ERR_PTR(-EINVAL);

	offset = offset_in_page(addr);
	count = DIV_ROUND_UP(offset + length, PAGE_SIZE);
	pages = kcalloc(count, sizeof(*pages), GFP_KERNEL);
	if (!pages)
		return ERR_PTR(-ENOMEM);

	err = get_user_pages_fast(addr, count, 1, pages);
	if (err < count) {
		count = err;
		err = -EFAULT;
		goto put_pages;
	}
{
	unsigned i;

	for (i = dev->queue_count; i <= dev->max_qid; i++)
		if (!nvme_alloc_queue(dev, i, dev->q_depth, i - 1))
			break;

	for (i = dev->online_queues; i <= dev->queue_count - 1; i++)
		if (nvme_create_queue(dev->queues[i], i))
			break;
}

static int set_queue_count(struct nvme_dev *dev, int count)
{
	int status;
	u32 result;
	u32 q_count = (count - 1) | ((count - 1) << 16);

	status = nvme_set_features(dev, NVME_FEAT_NUM_QUEUES, q_count, 0,
								&result);
	if (status < 0)
		return status;
	if (status > 0) {
		dev_err(&dev->pci_dev->dev, "Could not set queue count (%d)\n",
									status);
		return 0;
	}
	for (;;) {
		set_current_state(TASK_KILLABLE);
		if (!atomic_read(&dq->refcount))
			break;
		if (!schedule_timeout(ADMIN_TIMEOUT) ||
					fatal_signal_pending(current)) {
			set_current_state(TASK_RUNNING);

			nvme_disable_ctrl(dev, readq(&dev->bar->cap));
			nvme_disable_queue(dev, 0);

			send_sig(SIGKILL, dq->worker->task, 1);
			flush_kthread_worker(dq->worker);
			return;
		}
{
	struct nvme_queue *nvmeq = container_of(work, struct nvme_queue,
							cmdinfo.work);
	allow_signal(SIGKILL);
	if (nvme_delete_sq(nvmeq))
		nvme_del_queue_end(nvmeq);
}

static void nvme_disable_io_queues(struct nvme_dev *dev)
{
	int i;
	DEFINE_KTHREAD_WORKER_ONSTACK(worker);
	struct nvme_delq_ctx dq;
	struct task_struct *kworker_task = kthread_run(kthread_worker_fn,
					&worker, "nvme%d", dev->instance);

	if (IS_ERR(kworker_task)) {
		dev_err(&dev->pci_dev->dev,
			"Failed to create queue del task\n");
		for (i = dev->queue_count - 1; i > 0; i--)
			nvme_disable_queue(dev, i);
		return;
	}
	if (list_empty(&dev_list) && !IS_ERR_OR_NULL(nvme_thread)) {
		tmp = nvme_thread;
		nvme_thread = NULL;
	}
	spin_unlock(&dev_list_lock);

	if (tmp)
		kthread_stop(tmp);
}

static void nvme_dev_shutdown(struct nvme_dev *dev)
{
	int i;
	u32 csts = -1;

	dev->initialized = 0;
	nvme_dev_list_remove(dev);

	if (dev->bar)
		csts = readl(&dev->bar->csts);
	if (csts & NVME_CSTS_CFS || !(csts & NVME_CSTS_RDY)) {
		for (i = dev->queue_count - 1; i >= 0; i--) {
			struct nvme_queue *nvmeq = dev->queues[i];
			nvme_suspend_queue(nvmeq);
			nvme_clear_queue(nvmeq);
		}
	} else {
		nvme_disable_io_queues(dev);
		nvme_shutdown_ctrl(dev);
		nvme_disable_queue(dev, 0);
	}
	nvme_dev_unmap(dev);
}
{
	int i;
	u32 csts = -1;

	dev->initialized = 0;
	nvme_dev_list_remove(dev);

	if (dev->bar)
		csts = readl(&dev->bar->csts);
	if (csts & NVME_CSTS_CFS || !(csts & NVME_CSTS_RDY)) {
		for (i = dev->queue_count - 1; i >= 0; i--) {
			struct nvme_queue *nvmeq = dev->queues[i];
			nvme_suspend_queue(nvmeq);
			nvme_clear_queue(nvmeq);
		}
	} else {
		nvme_disable_io_queues(dev);
		nvme_shutdown_ctrl(dev);
		nvme_disable_queue(dev, 0);
	}
	nvme_dev_unmap(dev);
}
	} else {
		nvme_disable_io_queues(dev);
		nvme_shutdown_ctrl(dev);
		nvme_disable_queue(dev, 0);
	}
	nvme_dev_unmap(dev);
}

static void nvme_dev_remove_admin(struct nvme_dev *dev)
{
	if (dev->admin_q && !blk_queue_dying(dev->admin_q))
		blk_cleanup_queue(dev->admin_q);
}

static void nvme_dev_remove(struct nvme_dev *dev)
{
	struct nvme_ns *ns;

	list_for_each_entry(ns, &dev->namespaces, list) {
		if (ns->disk->flags & GENHD_FL_UP)
			del_gendisk(ns->disk);
		if (!blk_queue_dying(ns->queue))
			blk_cleanup_queue(ns->queue);
	}
{
	struct nvme_dev *dev = container_of(kref, struct nvme_dev, kref);

	pci_dev_put(dev->pci_dev);
	nvme_free_namespaces(dev);
	nvme_release_instance(dev);
	blk_mq_free_tag_set(&dev->tagset);
	kfree(dev->queues);
	kfree(dev->entry);
	kfree(dev);
}

static int nvme_dev_open(struct inode *inode, struct file *f)
{
	struct nvme_dev *dev = container_of(f->private_data, struct nvme_dev,
								miscdev);
	kref_get(&dev->kref);
	f->private_data = dev;
	return 0;
}

static int nvme_dev_release(struct inode *inode, struct file *f)
{
	struct nvme_dev *dev = f->private_data;
	kref_put(&dev->kref, nvme_free_dev);
	return 0;
}

static long nvme_dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	struct nvme_dev *dev = f->private_data;
	struct nvme_ns *ns;

	switch (cmd) {
	case NVME_IOCTL_ADMIN_CMD:
		return nvme_user_cmd(dev, NULL, (void __user *)arg);
	case NVME_IOCTL_IO_CMD:
		if (list_empty(&dev->namespaces))
			return -ENOTTY;
		ns = list_first_entry(&dev->namespaces, struct nvme_ns, list);
		return nvme_user_cmd(dev, ns, (void __user *)arg);
	default:
		return -ENOTTY;
	}
		result = nvme_thread ? PTR_ERR(nvme_thread) : -EINTR;
		goto disable;
	}

	nvme_init_queue(dev->queues[0], 0);

	result = nvme_setup_io_queues(dev);
	if (result)
		goto disable;

	nvme_set_irq_hints(dev);

	return result;

 disable:
	nvme_disable_queue(dev, 0);
	nvme_dev_list_remove(dev);
 unmap:
	nvme_dev_unmap(dev);
	return result;
}

static int nvme_remove_dead_ctrl(void *arg)
{
	struct nvme_dev *dev = (struct nvme_dev *)arg;
	struct pci_dev *pdev = dev->pci_dev;

	if (pci_get_drvdata(pdev))
		pci_stop_and_remove_bus_device_locked(pdev);
	kref_put(&dev->kref, nvme_free_dev);
	return 0;
}

static void nvme_remove_disks(struct work_struct *ws)
{
	struct nvme_dev *dev = container_of(ws, struct nvme_dev, reset_work);

	nvme_free_queues(dev, 1);
	nvme_dev_remove(dev);
}

static int nvme_dev_resume(struct nvme_dev *dev)
{
	int ret;

	ret = nvme_dev_start(dev);
	if (ret)
		return ret;
	if (dev->online_queues < 2) {
		spin_lock(&dev_list_lock);
		dev->reset_workfn = nvme_remove_disks;
		queue_work(nvme_workq, &dev->reset_work);
		spin_unlock(&dev_list_lock);
	}
	if (dev->online_queues < 2) {
		spin_lock(&dev_list_lock);
		dev->reset_workfn = nvme_remove_disks;
		queue_work(nvme_workq, &dev->reset_work);
		spin_unlock(&dev_list_lock);
	}
	dev->initialized = 1;
	return 0;
}

static void nvme_dev_reset(struct nvme_dev *dev)
{
	nvme_dev_shutdown(dev);
	if (nvme_dev_resume(dev)) {
		dev_warn(&dev->pci_dev->dev, "Device failed to resume\n");
		kref_get(&dev->kref);
		if (IS_ERR(kthread_run(nvme_remove_dead_ctrl, dev, "nvme%d",
							dev->instance))) {
			dev_err(&dev->pci_dev->dev,
				"Failed to start controller remove task\n");
			kref_put(&dev->kref, nvme_free_dev);
		}
{
	struct nvme_dev *dev = pci_get_drvdata(pdev);

	spin_lock(&dev_list_lock);
	list_del_init(&dev->node);
	spin_unlock(&dev_list_lock);

	pci_set_drvdata(pdev, NULL);
	flush_work(&dev->reset_work);
	misc_deregister(&dev->miscdev);
	nvme_dev_remove(dev);
	nvme_dev_shutdown(dev);
	nvme_dev_remove_admin(dev);
	nvme_free_queues(dev, 0);
	nvme_free_admin_tags(dev);
	nvme_release_prp_pools(dev);
	kref_put(&dev->kref, nvme_free_dev);
}

/* These functions are yet to be implemented */
#define nvme_error_detected NULL
#define nvme_dump_registers NULL
#define nvme_link_reset NULL
#define nvme_slot_reset NULL
#define nvme_error_resume NULL

#ifdef CONFIG_PM_SLEEP
static int nvme_suspend(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct nvme_dev *ndev = pci_get_drvdata(pdev);

	nvme_dev_shutdown(ndev);
	return 0;
}

static int nvme_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct nvme_dev *ndev = pci_get_drvdata(pdev);

	if (nvme_dev_resume(ndev) && !work_busy(&ndev->reset_work)) {
		ndev->reset_workfn = nvme_reset_failed_dev;
		queue_work(nvme_workq, &ndev->reset_work);
	}