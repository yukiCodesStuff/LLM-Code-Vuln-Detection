	struct futex_q *this, *next;

	if (requeue_pi) {
		/*
		 * Requeue PI only works on two distinct uaddrs. This
		 * check is only valid for private futexes. See below.
		 */
		if (uaddr1 == uaddr2)
			return -EINVAL;

		/*
		 * requeue_pi requires a pi_state, try to allocate it now
		 * without any locks in case it fails.
		 */
	if (unlikely(ret != 0))
		goto out_put_key1;

	/*
	 * The check above which compares uaddrs is not sufficient for
	 * shared futexes. We need to compare the keys:
	 */
	if (requeue_pi && match_futex(&key1, &key2)) {
		ret = -EINVAL;
		goto out_put_keys;
	}

	hb1 = hash_futex(&key1);
	hb2 = hash_futex(&key2);

retry_private:
	if (ret)
		goto out_key2;

	/*
	 * The check above which compares uaddrs is not sufficient for
	 * shared futexes. We need to compare the keys:
	 */
	if (match_futex(&q.key, &key2)) {
		ret = -EINVAL;
		goto out_put_keys;
	}

	/* Queue the futex_q, drop the hb lock, wait for wakeup. */
	futex_wait_queue_me(hb, &q, to);

	spin_lock(&hb->lock);