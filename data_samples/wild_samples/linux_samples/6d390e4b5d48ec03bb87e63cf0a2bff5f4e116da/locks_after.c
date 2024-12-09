{
	int status = -ENOENT;

	spin_lock(&blocked_lock_lock);
	if (waiter->fl_blocker)
		status = 0;
	__locks_wake_up_blocks(waiter);
	__locks_delete_block(waiter);
	spin_unlock(&blocked_lock_lock);
	return status;
}
EXPORT_SYMBOL(locks_delete_block);

/* Insert waiter into blocker's block list.
 * We use a circular list so that processes can be easily woken up in
 * the order they blocked. The documentation doesn't require this but
 * it seems like the reasonable thing to do.
 *
 * Must be called with both the flc_lock and blocked_lock_lock held. The
 * fl_blocked_requests list itself is protected by the blocked_lock_lock,
 * but by ensuring that the flc_lock is also held on insertions we can avoid
 * taking the blocked_lock_lock in some cases when we see that the
 * fl_blocked_requests list is empty.
 *
 * Rather than just adding to the list, we check for conflicts with any existing
 * waiters, and add beneath any waiter that blocks the new waiter.
 * Thus wakeups don't happen until needed.
 */
static void __locks_insert_block(struct file_lock *blocker,
				 struct file_lock *waiter,
				 bool conflict(struct file_lock *,
					       struct file_lock *))
{
	struct file_lock *fl;
	BUG_ON(!list_empty(&waiter->fl_blocked_member));

new_blocker:
	list_for_each_entry(fl, &blocker->fl_blocked_requests, fl_blocked_member)
		if (conflict(fl, waiter)) {
			blocker =  fl;
			goto new_blocker;
		}