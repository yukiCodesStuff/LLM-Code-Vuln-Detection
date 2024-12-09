		if (test_bit(KEY_FLAG_IN_QUOTA, &key->flags)) {
			spin_lock(&key->user->lock);
			key->user->qnkeys--;
			key->user->qnbytes -= key->quotalen;
			spin_unlock(&key->user->lock);
		}

		atomic_dec(&key->user->nkeys);
		if (test_bit(KEY_FLAG_INSTANTIATED, &key->flags))
			atomic_dec(&key->user->nikeys);

		key_user_put(key->user);

		/* now throw away the key memory */
		if (key->type->destroy)
			key->type->destroy(key);

		kfree(key->description);

#ifdef KEY_DEBUGGING
		key->magic = KEY_DEBUG_MAGIC_X;
#endif
		kmem_cache_free(key_jar, key);
	}
}

/*
 * Garbage collector for unused keys.
 *
 * This is done in process context so that we don't have to disable interrupts
 * all over the place.  key_put() schedules this rather than trying to do the
 * cleanup itself, which means key_put() doesn't have to sleep.
 */
static void key_garbage_collector(struct work_struct *work)
{
	static LIST_HEAD(graveyard);
	static u8 gc_state;		/* Internal persistent state */
#define KEY_GC_REAP_AGAIN	0x01	/* - Need another cycle */
#define KEY_GC_REAPING_LINKS	0x02	/* - We need to reap links */
#define KEY_GC_SET_TIMER	0x04	/* - We need to restart the timer */
#define KEY_GC_REAPING_DEAD_1	0x10	/* - We need to mark dead keys */
#define KEY_GC_REAPING_DEAD_2	0x20	/* - We need to reap dead key links */
#define KEY_GC_REAPING_DEAD_3	0x40	/* - We need to reap dead keys */
#define KEY_GC_FOUND_DEAD_KEY	0x80	/* - We found at least one dead key */

	struct rb_node *cursor;
	struct key *key;
	time_t new_timer, limit;

	kenter("[%lx,%x]", key_gc_flags, gc_state);

	limit = current_kernel_time().tv_sec;
	if (limit > key_gc_delay)
		limit -= key_gc_delay;
	else
		limit = key_gc_delay;

	/* Work out what we're going to be doing in this pass */
	gc_state &= KEY_GC_REAPING_DEAD_1 | KEY_GC_REAPING_DEAD_2;
	gc_state <<= 1;
	if (test_and_clear_bit(KEY_GC_KEY_EXPIRED, &key_gc_flags))
		gc_state |= KEY_GC_REAPING_LINKS | KEY_GC_SET_TIMER;

	if (test_and_clear_bit(KEY_GC_REAP_KEYTYPE, &key_gc_flags))
		gc_state |= KEY_GC_REAPING_DEAD_1;
	kdebug("new pass %x", gc_state);

	new_timer = LONG_MAX;

	/* As only this function is permitted to remove things from the key
	 * serial tree, if cursor is non-NULL then it will always point to a
	 * valid node in the tree - even if lock got dropped.
	 */
	spin_lock(&key_serial_lock);
	cursor = rb_first(&key_serial_tree);

continue_scanning:
	while (cursor) {
		key = rb_entry(cursor, struct key, serial_node);
		cursor = rb_next(cursor);

		if (atomic_read(&key->usage) == 0)
			goto found_unreferenced_key;

		if (unlikely(gc_state & KEY_GC_REAPING_DEAD_1)) {
			if (key->type == key_gc_dead_keytype) {
				gc_state |= KEY_GC_FOUND_DEAD_KEY;
				set_bit(KEY_FLAG_DEAD, &key->flags);
				key->perm = 0;
				goto skip_dead_key;
			}
		}