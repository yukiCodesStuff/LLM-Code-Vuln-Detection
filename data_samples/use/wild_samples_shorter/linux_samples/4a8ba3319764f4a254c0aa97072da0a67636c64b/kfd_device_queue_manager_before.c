{
	int bit = qpd->vmid - KFD_VMID_START_OFFSET;

	set_bit(bit, (unsigned long *)&dqm->vmid_bitmap);
	qpd->vmid = 0;
	q->properties.vmid = 0;
}
		return retval;
	}

	return 0;
}

static int destroy_queue_nocpsch(struct device_queue_manager *dqm,
{
	int retval;
	struct mqd_manager *mqd;

	BUG_ON(!dqm || !q || !q->mqd);

	mutex_lock(&dqm->lock);
		return -ENOMEM;
	}

	retval = mqd->update_mqd(mqd, q->mqd, &q->properties);
	if (q->properties.is_active == true)
		dqm->queue_count++;
	else
		dqm->queue_count--;

	if (sched_policy != KFD_SCHED_POLICY_NO_HWS)
		retval = execute_queues_cpsch(dqm, false);