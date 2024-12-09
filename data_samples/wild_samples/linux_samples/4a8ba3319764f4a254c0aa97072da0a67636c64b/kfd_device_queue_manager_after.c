{
	int bit = qpd->vmid - KFD_VMID_START_OFFSET;

	/* Release the vmid mapping */
	set_pasid_vmid_mapping(dqm, 0, qpd->vmid);

	set_bit(bit, (unsigned long *)&dqm->vmid_bitmap);
	qpd->vmid = 0;
	q->properties.vmid = 0;
}

static int create_queue_nocpsch(struct device_queue_manager *dqm,
				struct queue *q,
				struct qcm_process_device *qpd,
				int *allocated_vmid)
{
	int retval;

	BUG_ON(!dqm || !q || !qpd || !allocated_vmid);

	pr_debug("kfd: In func %s\n", __func__);
	print_queue(q);

	mutex_lock(&dqm->lock);

	if (list_empty(&qpd->queues_list)) {
		retval = allocate_vmid(dqm, qpd, q);
		if (retval != 0) {
			mutex_unlock(&dqm->lock);
			return retval;
		}
	}
	if (retval != 0) {
		deallocate_hqd(dqm, q);
		return retval;
	}

	pr_debug("kfd: loading mqd to hqd on pipe (%d) queue (%d)\n",
			q->pipe,
			q->queue);

	retval = mqd->load_mqd(mqd, q->mqd, q->pipe,
			q->queue, (uint32_t __user *) q->properties.write_ptr);
	if (retval != 0) {
		deallocate_hqd(dqm, q);
		mqd->uninit_mqd(mqd, q->mqd, q->mqd_mem_obj);
		return retval;
	}

	return 0;
}

static int destroy_queue_nocpsch(struct device_queue_manager *dqm,
				struct qcm_process_device *qpd,
				struct queue *q)
{
	int retval;
	struct mqd_manager *mqd;

	BUG_ON(!dqm || !q || !q->mqd || !qpd);

	retval = 0;

	pr_debug("kfd: In Func %s\n", __func__);

	mutex_lock(&dqm->lock);
	mqd = dqm->get_mqd_manager(dqm, KFD_MQD_TYPE_CIK_COMPUTE);
	if (mqd == NULL) {
		retval = -ENOMEM;
		goto out;
	}

	retval = mqd->destroy_mqd(mqd, q->mqd,
				KFD_PREEMPT_TYPE_WAVEFRONT,
				QUEUE_PREEMPT_DEFAULT_TIMEOUT_MS,
				q->pipe, q->queue);

	if (retval != 0)
		goto out;

	deallocate_hqd(dqm, q);

	mqd->uninit_mqd(mqd, q->mqd, q->mqd_mem_obj);

	list_del(&q->list);
	if (list_empty(&qpd->queues_list))
		deallocate_vmid(dqm, qpd, q);
	dqm->queue_count--;
out:
	mutex_unlock(&dqm->lock);
	return retval;
}

static int update_queue(struct device_queue_manager *dqm, struct queue *q)
{
	int retval;
	struct mqd_manager *mqd;
	bool prev_active = false;

	BUG_ON(!dqm || !q || !q->mqd);

	mutex_lock(&dqm->lock);
	mqd = dqm->get_mqd_manager(dqm, KFD_MQD_TYPE_CIK_COMPUTE);
	if (mqd == NULL) {
		mutex_unlock(&dqm->lock);
		return -ENOMEM;
	}

	if (q->properties.is_active == true)
		prev_active = true;

	/*
	 *
	 * check active state vs. the previous state
	 * and modify counter accordingly
	 */
	retval = mqd->update_mqd(mqd, q->mqd, &q->properties);
	if ((q->properties.is_active == true) && (prev_active == false))
		dqm->queue_count++;
	else if ((q->properties.is_active == false) && (prev_active == true))
		dqm->queue_count--;

	if (sched_policy != KFD_SCHED_POLICY_NO_HWS)
		retval = execute_queues_cpsch(dqm, false);

	mutex_unlock(&dqm->lock);
	return retval;
}

static struct mqd_manager *get_mqd_manager_nocpsch(
		struct device_queue_manager *dqm, enum KFD_MQD_TYPE type)
{
	struct mqd_manager *mqd;

	BUG_ON(!dqm || type >= KFD_MQD_TYPE_MAX);

	pr_debug("kfd: In func %s mqd type %d\n", __func__, type);

	mqd = dqm->mqds[type];
	if (!mqd) {
		mqd = mqd_manager_init(type, dqm->dev);
		if (mqd == NULL)
			pr_err("kfd: mqd manager is NULL");
		dqm->mqds[type] = mqd;
	}

	return mqd;
}

static int register_process_nocpsch(struct device_queue_manager *dqm,
					struct qcm_process_device *qpd)
{
	struct device_process_node *n;

	BUG_ON(!dqm || !qpd);

	pr_debug("kfd: In func %s\n", __func__);

	n = kzalloc(sizeof(struct device_process_node), GFP_KERNEL);
	if (!n)
		return -ENOMEM;

	n->qpd = qpd;

	mutex_lock(&dqm->lock);
	list_add(&n->list, &dqm->queues);

	init_process_memory(dqm, qpd);
	dqm->processes_count++;

	mutex_unlock(&dqm->lock);

	return 0;
}

static int unregister_process_nocpsch(struct device_queue_manager *dqm,
					struct qcm_process_device *qpd)
{
	int retval;
	struct device_process_node *cur, *next;

	BUG_ON(!dqm || !qpd);

	BUG_ON(!list_empty(&qpd->queues_list));

	pr_debug("kfd: In func %s\n", __func__);

	retval = 0;
	mutex_lock(&dqm->lock);

	list_for_each_entry_safe(cur, next, &dqm->queues, list) {
		if (qpd == cur->qpd) {
			list_del(&cur->list);
			kfree(cur);
			dqm->processes_count--;
			goto out;
		}
{
	int retval;
	struct mqd_manager *mqd;
	bool prev_active = false;

	BUG_ON(!dqm || !q || !q->mqd);

	mutex_lock(&dqm->lock);
	mqd = dqm->get_mqd_manager(dqm, KFD_MQD_TYPE_CIK_COMPUTE);
	if (mqd == NULL) {
		mutex_unlock(&dqm->lock);
		return -ENOMEM;
	}
	if (mqd == NULL) {
		mutex_unlock(&dqm->lock);
		return -ENOMEM;
	}

	if (q->properties.is_active == true)
		prev_active = true;

	/*
	 *
	 * check active state vs. the previous state
	 * and modify counter accordingly
	 */
	retval = mqd->update_mqd(mqd, q->mqd, &q->properties);
	if ((q->properties.is_active == true) && (prev_active == false))
		dqm->queue_count++;
	else if ((q->properties.is_active == false) && (prev_active == true))
		dqm->queue_count--;

	if (sched_policy != KFD_SCHED_POLICY_NO_HWS)
		retval = execute_queues_cpsch(dqm, false);

	mutex_unlock(&dqm->lock);
	return retval;
}

static struct mqd_manager *get_mqd_manager_nocpsch(
		struct device_queue_manager *dqm, enum KFD_MQD_TYPE type)
{
	struct mqd_manager *mqd;

	BUG_ON(!dqm || type >= KFD_MQD_TYPE_MAX);

	pr_debug("kfd: In func %s mqd type %d\n", __func__, type);

	mqd = dqm->mqds[type];
	if (!mqd) {
		mqd = mqd_manager_init(type, dqm->dev);
		if (mqd == NULL)
			pr_err("kfd: mqd manager is NULL");
		dqm->mqds[type] = mqd;
	}

	return mqd;
}

static int register_process_nocpsch(struct device_queue_manager *dqm,
					struct qcm_process_device *qpd)
{
	struct device_process_node *n;

	BUG_ON(!dqm || !qpd);

	pr_debug("kfd: In func %s\n", __func__);

	n = kzalloc(sizeof(struct device_process_node), GFP_KERNEL);
	if (!n)
		return -ENOMEM;

	n->qpd = qpd;

	mutex_lock(&dqm->lock);
	list_add(&n->list, &dqm->queues);

	init_process_memory(dqm, qpd);
	dqm->processes_count++;

	mutex_unlock(&dqm->lock);

	return 0;
}

static int unregister_process_nocpsch(struct device_queue_manager *dqm,
					struct qcm_process_device *qpd)
{
	int retval;
	struct device_process_node *cur, *next;

	BUG_ON(!dqm || !qpd);

	BUG_ON(!list_empty(&qpd->queues_list));

	pr_debug("kfd: In func %s\n", __func__);

	retval = 0;
	mutex_lock(&dqm->lock);

	list_for_each_entry_safe(cur, next, &dqm->queues, list) {
		if (qpd == cur->qpd) {
			list_del(&cur->list);
			kfree(cur);
			dqm->processes_count--;
			goto out;
		}