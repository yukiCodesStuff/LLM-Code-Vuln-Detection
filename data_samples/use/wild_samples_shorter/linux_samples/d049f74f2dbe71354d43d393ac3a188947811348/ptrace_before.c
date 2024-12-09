	if (task->mm)
		dumpable = get_dumpable(task->mm);
	rcu_read_lock();
	if (!dumpable && !ptrace_has_cap(__task_cred(task)->user_ns, mode)) {
		rcu_read_unlock();
		return -EPERM;
	}
	rcu_read_unlock();