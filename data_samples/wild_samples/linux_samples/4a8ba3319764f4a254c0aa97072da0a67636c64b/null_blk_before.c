	if (queue_mode == NULL_Q_MQ) {
		nullb->tag_set.ops = &null_mq_ops;
		nullb->tag_set.nr_hw_queues = submit_queues;
		nullb->tag_set.queue_depth = hw_queue_depth;
		nullb->tag_set.numa_node = home_node;
		nullb->tag_set.cmd_size	= sizeof(struct nullb_cmd);
		nullb->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
		nullb->tag_set.driver_data = nullb;

		rv = blk_mq_alloc_tag_set(&nullb->tag_set);
		if (rv)
			goto out_cleanup_queues;

		nullb->q = blk_mq_init_queue(&nullb->tag_set);
		if (!nullb->q) {
			rv = -ENOMEM;
			goto out_cleanup_tags;
		}