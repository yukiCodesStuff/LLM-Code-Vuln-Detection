	if (!is_key_possessed(key_ref)) {
		ret = -EACCES;
		goto error2;
	}

	/* the key is probably readable - now try to read it */
can_read_key:
	ret = -EOPNOTSUPP;
	if (key->type->read) {
		/* Read the data with the semaphore held (since we might sleep)
		 * to protect against the key being updated or revoked.
		 */
		down_read(&key->sem);
		ret = key_validate(key);
		if (ret == 0)
			ret = key->type->read(key, buffer, buflen);
		up_read(&key->sem);
	}