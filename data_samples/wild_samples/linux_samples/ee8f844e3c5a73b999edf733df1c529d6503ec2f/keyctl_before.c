	if (IS_ERR(key_ref)) {
		ret = PTR_ERR(key_ref);
		goto error;
	}

	ret = key_ref_to_ptr(key_ref)->serial;
	key_ref_put(key_ref);
error:
	return ret;
}

/*
 * Join a (named) session keyring.
 *
 * Create and join an anonymous session keyring or join a named session
 * keyring, creating it if necessary.  A named session keyring must have Search
 * permission for it to be joined.  Session keyrings without this permit will
 * be skipped over.
 *
 * If successful, the ID of the joined session keyring will be returned.
 */
long keyctl_join_session_keyring(const char __user *_name)
{
	char *name;
	long ret;

	/* fetch the name from userspace */
	name = NULL;
	if (_name) {
		name = strndup_user(_name, KEY_MAX_DESC_SIZE);
		if (IS_ERR(name)) {
			ret = PTR_ERR(name);
			goto error;
		}
	}

	/* join the session */
	ret = join_session_keyring(name);
	kfree(name);

error:
	return ret;
}
		if (IS_ERR(name)) {
			ret = PTR_ERR(name);
			goto error;
		}
	}

	/* join the session */
	ret = join_session_keyring(name);
	kfree(name);

error:
	return ret;
}

/*
 * Update a key's data payload from the given data.
 *
 * The key must grant the caller Write permission and the key type must support
 * updating for this to work.  A negative key can be positively instantiated
 * with this call.
 *
 * If successful, 0 will be returned.  If the key type does not support
 * updating, then -EOPNOTSUPP will be returned.
 */
long keyctl_update_key(key_serial_t id,
		       const void __user *_payload,
		       size_t plen)
{
	key_ref_t key_ref;
	void *payload;
	long ret;

	ret = -EINVAL;
	if (plen > PAGE_SIZE)
		goto error;

	/* pull the payload in if one was supplied */
	payload = NULL;
	if (_payload) {
		ret = -ENOMEM;
		payload = kmalloc(plen, GFP_KERNEL);
		if (!payload)
			goto error;

		ret = -EFAULT;
		if (copy_from_user(payload, _payload, plen) != 0)
			goto error2;
	}

	/* find the target key (which must be writable) */
	key_ref = lookup_user_key(id, 0, KEY_NEED_WRITE);
	if (IS_ERR(key_ref)) {
		ret = PTR_ERR(key_ref);
		goto error2;
	}

	/* update the key */
	ret = key_update(key_ref, payload, plen);

	key_ref_put(key_ref);
error2:
	kfree(payload);
error:
	return ret;
}

/*
 * Revoke a key.
 *
 * The key must be grant the caller Write or Setattr permission for this to
 * work.  The key type should give up its quota claim when revoked.  The key
 * and any links to the key will be automatically garbage collected after a
 * certain amount of time (/proc/sys/kernel/keys/gc_delay).
 *
 * Keys with KEY_FLAG_KEEP set should not be revoked.
 *
 * If successful, 0 is returned.
 */
long keyctl_revoke_key(key_serial_t id)
{
	key_ref_t key_ref;
	struct key *key;
	long ret;

	key_ref = lookup_user_key(id, 0, KEY_NEED_WRITE);
	if (IS_ERR(key_ref)) {
		ret = PTR_ERR(key_ref);
		if (ret != -EACCES)
			goto error;
		key_ref = lookup_user_key(id, 0, KEY_NEED_SETATTR);
		if (IS_ERR(key_ref)) {
			ret = PTR_ERR(key_ref);
			goto error;
		}
	}

	key = key_ref_to_ptr(key_ref);
	ret = 0;
	if (test_bit(KEY_FLAG_KEEP, &key->flags))
		ret = -EPERM;
	else
		key_revoke(key);

	key_ref_put(key_ref);
error:
	return ret;
}

/*
 * Invalidate a key.
 *
 * The key must be grant the caller Invalidate permission for this to work.
 * The key and any links to the key will be automatically garbage collected
 * immediately.
 *
 * Keys with KEY_FLAG_KEEP set should not be invalidated.
 *
 * If successful, 0 is returned.
 */
long keyctl_invalidate_key(key_serial_t id)
{
	key_ref_t key_ref;
	struct key *key;
	long ret;

	kenter("%d", id);

	key_ref = lookup_user_key(id, 0, KEY_NEED_SEARCH);
	if (IS_ERR(key_ref)) {
		ret = PTR_ERR(key_ref);

		/* Root is permitted to invalidate certain special keys */
		if (capable(CAP_SYS_ADMIN)) {
			key_ref = lookup_user_key(id, 0, 0);
			if (IS_ERR(key_ref))
				goto error;
			if (test_bit(KEY_FLAG_ROOT_CAN_INVAL,
				     &key_ref_to_ptr(key_ref)->flags))
				goto invalidate;
			goto error_put;
		}

		goto error;
	}

invalidate:
	key = key_ref_to_ptr(key_ref);
	ret = 0;
	if (test_bit(KEY_FLAG_KEEP, &key->flags))
		ret = -EPERM;
	else
		key_invalidate(key);
error_put:
	key_ref_put(key_ref);
error:
	kleave(" = %ld", ret);
	return ret;
}

/*
 * Clear the specified keyring, creating an empty process keyring if one of the
 * special keyring IDs is used.
 *
 * The keyring must grant the caller Write permission and not have
 * KEY_FLAG_KEEP set for this to work.  If successful, 0 will be returned.
 */
long keyctl_keyring_clear(key_serial_t ringid)
{
	key_ref_t keyring_ref;
	struct key *keyring;
	long ret;

	keyring_ref = lookup_user_key(ringid, KEY_LOOKUP_CREATE, KEY_NEED_WRITE);
	if (IS_ERR(keyring_ref)) {
		ret = PTR_ERR(keyring_ref);

		/* Root is permitted to invalidate certain special keyrings */
		if (capable(CAP_SYS_ADMIN)) {
			keyring_ref = lookup_user_key(ringid, 0, 0);
			if (IS_ERR(keyring_ref))
				goto error;
			if (test_bit(KEY_FLAG_ROOT_CAN_CLEAR,
				     &key_ref_to_ptr(keyring_ref)->flags))
				goto clear;
			goto error_put;
		}

		goto error;
	}

clear:
	keyring = key_ref_to_ptr(keyring_ref);
	if (test_bit(KEY_FLAG_KEEP, &keyring->flags))
		ret = -EPERM;
	else
		ret = keyring_clear(keyring);
error_put:
	key_ref_put(keyring_ref);
error:
	return ret;
}

/*
 * Create a link from a keyring to a key if there's no matching key in the
 * keyring, otherwise replace the link to the matching key with a link to the
 * new key.
 *
 * The key must grant the caller Link permission and the the keyring must grant
 * the caller Write permission.  Furthermore, if an additional link is created,
 * the keyring's quota will be extended.
 *
 * If successful, 0 will be returned.
 */
long keyctl_keyring_link(key_serial_t id, key_serial_t ringid)
{
	key_ref_t keyring_ref, key_ref;
	long ret;

	keyring_ref = lookup_user_key(ringid, KEY_LOOKUP_CREATE, KEY_NEED_WRITE);
	if (IS_ERR(keyring_ref)) {
		ret = PTR_ERR(keyring_ref);
		goto error;
	}

	key_ref = lookup_user_key(id, KEY_LOOKUP_CREATE, KEY_NEED_LINK);
	if (IS_ERR(key_ref)) {
		ret = PTR_ERR(key_ref);
		goto error2;
	}

	ret = key_link(key_ref_to_ptr(keyring_ref), key_ref_to_ptr(key_ref));

	key_ref_put(key_ref);
error2:
	key_ref_put(keyring_ref);
error:
	return ret;
}

/*
 * Unlink a key from a keyring.
 *
 * The keyring must grant the caller Write permission for this to work; the key
 * itself need not grant the caller anything.  If the last link to a key is
 * removed then that key will be scheduled for destruction.
 *
 * Keys or keyrings with KEY_FLAG_KEEP set should not be unlinked.
 *
 * If successful, 0 will be returned.
 */
long keyctl_keyring_unlink(key_serial_t id, key_serial_t ringid)
{
	key_ref_t keyring_ref, key_ref;
	struct key *keyring, *key;
	long ret;

	keyring_ref = lookup_user_key(ringid, 0, KEY_NEED_WRITE);
	if (IS_ERR(keyring_ref)) {
		ret = PTR_ERR(keyring_ref);
		goto error;
	}

	key_ref = lookup_user_key(id, KEY_LOOKUP_FOR_UNLINK, 0);
	if (IS_ERR(key_ref)) {
		ret = PTR_ERR(key_ref);
		goto error2;
	}

	keyring = key_ref_to_ptr(keyring_ref);
	key = key_ref_to_ptr(key_ref);
	if (test_bit(KEY_FLAG_KEEP, &keyring->flags) &&
	    test_bit(KEY_FLAG_KEEP, &key->flags))
		ret = -EPERM;
	else
		ret = key_unlink(keyring, key);

	key_ref_put(key_ref);
error2:
	key_ref_put(keyring_ref);
error:
	return ret;
}

/*
 * Return a description of a key to userspace.
 *
 * The key must grant the caller View permission for this to work.
 *
 * If there's a buffer, we place up to buflen bytes of data into it formatted
 * in the following way:
 *
 *	type;uid;gid;perm;description<NUL>
 *
 * If successful, we return the amount of description available, irrespective
 * of how much we may have copied into the buffer.
 */
long keyctl_describe_key(key_serial_t keyid,
			 char __user *buffer,
			 size_t buflen)
{
	struct key *key, *instkey;
	key_ref_t key_ref;
	char *infobuf;
	long ret;
	int desclen, infolen;

	key_ref = lookup_user_key(keyid, KEY_LOOKUP_PARTIAL, KEY_NEED_VIEW);
	if (IS_ERR(key_ref)) {
		/* viewing a key under construction is permitted if we have the
		 * authorisation token handy */
		if (PTR_ERR(key_ref) == -EACCES) {
			instkey = key_get_instantiation_authkey(keyid);
			if (!IS_ERR(instkey)) {
				key_put(instkey);
				key_ref = lookup_user_key(keyid,
							  KEY_LOOKUP_PARTIAL,
							  0);
				if (!IS_ERR(key_ref))
					goto okay;
			}