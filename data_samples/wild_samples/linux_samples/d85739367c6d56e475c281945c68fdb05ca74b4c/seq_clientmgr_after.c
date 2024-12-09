	     info->output_pool != client->pool->size)) {
		if (snd_seq_write_pool_allocated(client)) {
			/* is the pool in use? */
			if (atomic_read(&client->pool->counter))
				return -EBUSY;
			/* remove all existing cells */
			snd_seq_pool_mark_closing(client->pool);
			snd_seq_queue_client_leave_cells(client->number);
			snd_seq_pool_done(client->pool);
		}
		client->pool->size = info->output_pool;
		rc = snd_seq_pool_init(client->pool);
		if (rc < 0)
			return rc;
	}