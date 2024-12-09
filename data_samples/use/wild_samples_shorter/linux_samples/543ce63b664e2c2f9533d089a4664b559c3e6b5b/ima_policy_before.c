	if (id >= READING_MAX_ID)
		return false;

	func = read_idmap[id] ?: FILE_CHECK;

	rcu_read_lock();
	ima_rules_tmp = rcu_dereference(ima_rules);