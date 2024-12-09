{
	int bit = qpd->vmid - KFD_VMID_START_OFFSET;

	/* Release the vmid mapping */
	set_pasid_vmid_mapping(dqm, 0, qpd->vmid);

	set_bit(bit, (unsigned long *)&dqm->vmid_bitmap);
	qpd->vmid = 0;
	q->properties.vmid = 0;
}
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
{
	int retval;
	struct mqd_manager *mqd;
	bool prev_active = false;

	BUG_ON(!dqm || !q || !q->mqd);

	mutex_lock(&dqm->lock);
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