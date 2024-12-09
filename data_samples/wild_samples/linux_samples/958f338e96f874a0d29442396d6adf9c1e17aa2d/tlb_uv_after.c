{
	int count = 0;
	cycles_t time_start;
	struct bau_pq_entry *msg;
	struct bau_control *bcp;
	struct ptc_stats *stat;
	struct msg_desc msgdesc;

	ack_APIC_irq();
	kvm_set_cpu_l1tf_flush_l1d();
	time_start = get_cycles();

	bcp = &per_cpu(bau_control, smp_processor_id());
	stat = bcp->statp;

	msgdesc.queue_first = bcp->queue_first;
	msgdesc.queue_last = bcp->queue_last;

	msg = bcp->bau_msg_head;
	while (msg->swack_vec) {
		count++;

		msgdesc.msg_slot = msg - msgdesc.queue_first;
		msgdesc.msg = msg;
		if (bcp->uvhub_version == UV_BAU_V2)
			process_uv2_message(&msgdesc, bcp);
		else
			/* no error workaround for uv1 or uv3 */
			bau_process_message(&msgdesc, bcp, 1);

		msg++;
		if (msg > msgdesc.queue_last)
			msg = msgdesc.queue_first;
		bcp->bau_msg_head = msg;
	}