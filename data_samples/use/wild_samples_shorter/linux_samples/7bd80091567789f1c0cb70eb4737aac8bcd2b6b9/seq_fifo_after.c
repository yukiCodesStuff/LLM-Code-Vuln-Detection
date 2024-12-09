		return -EINVAL;

	snd_use_lock_use(&f->use_lock);
	err = snd_seq_event_dup(f->pool, event, &cell, 1, NULL, NULL); /* always non-blocking */
	if (err < 0) {
		if ((err == -ENOMEM) || (err == -EAGAIN))
			atomic_inc(&f->overflow);
		snd_use_lock_free(&f->use_lock);