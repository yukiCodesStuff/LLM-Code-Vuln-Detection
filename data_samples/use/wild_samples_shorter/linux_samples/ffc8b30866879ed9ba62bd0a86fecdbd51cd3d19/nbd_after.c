		else
			blk_queue_flush(nbd->disk->queue, 0);

		thread = kthread_create(nbd_thread, nbd, "%s",
					nbd->disk->disk_name);
		if (IS_ERR(thread)) {
			mutex_lock(&nbd->tx_lock);
			return PTR_ERR(thread);
		}