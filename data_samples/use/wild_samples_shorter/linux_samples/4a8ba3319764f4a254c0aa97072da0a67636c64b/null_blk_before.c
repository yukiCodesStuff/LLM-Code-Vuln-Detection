			goto out_cleanup_queues;

		nullb->q = blk_mq_init_queue(&nullb->tag_set);
		if (!nullb->q) {
			rv = -ENOMEM;
			goto out_cleanup_tags;
		}
	} else if (queue_mode == NULL_Q_BIO) {