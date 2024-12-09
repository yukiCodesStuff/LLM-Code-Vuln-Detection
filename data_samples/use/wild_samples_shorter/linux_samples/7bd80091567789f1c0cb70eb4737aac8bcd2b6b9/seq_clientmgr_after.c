static int snd_seq_client_enqueue_event(struct snd_seq_client *client,
					struct snd_seq_event *event,
					struct file *file, int blocking,
					int atomic, int hop,
					struct mutex *mutexp)
{
	struct snd_seq_event_cell *cell;
	int err;

		return -ENXIO; /* queue is not allocated */

	/* allocate an event cell */
	err = snd_seq_event_dup(client->pool, event, &cell, !blocking || atomic,
				file, mutexp);
	if (err < 0)
		return err;

	/* we got a cell. enqueue it. */
		return -ENXIO;

	/* allocate the pool now if the pool is not allocated yet */ 
	mutex_lock(&client->ioctl_mutex);
	if (client->pool->size > 0 && !snd_seq_write_pool_allocated(client)) {
		err = snd_seq_pool_init(client->pool);
		if (err < 0)
			goto out;
	}

	/* only process whole events */
	err = -EINVAL;
		/* ok, enqueue it */
		err = snd_seq_client_enqueue_event(client, &event, file,
						   !(file->f_flags & O_NONBLOCK),
						   0, 0, &client->ioctl_mutex);
		if (err < 0)
			break;

	__skip_event:
		written += len;
	}

 out:
	mutex_unlock(&client->ioctl_mutex);
	return written ? written : err;
}


	if (! cptr->accept_output)
		result = -EPERM;
	else /* send it */
		result = snd_seq_client_enqueue_event(cptr, ev, file, blocking,
						      atomic, hop, NULL);

	snd_seq_client_unlock(cptr);
	return result;
}