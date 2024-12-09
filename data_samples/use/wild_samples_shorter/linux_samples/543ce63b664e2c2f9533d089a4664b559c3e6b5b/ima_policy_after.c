	if (id >= READING_MAX_ID)
		return false;

	if (id == READING_KEXEC_IMAGE && !(ima_appraise & IMA_APPRAISE_ENFORCE)
	    && security_locked_down(LOCKDOWN_KEXEC))
		return false;

	func = read_idmap[id] ?: FILE_CHECK;

	rcu_read_lock();
	ima_rules_tmp = rcu_dereference(ima_rules);