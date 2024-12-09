{
	struct ima_rule_entry *entry;
	bool found = false;
	enum ima_hooks func;
	struct list_head *ima_rules_tmp;

	if (id >= READING_MAX_ID)
		return false;

	if (id == READING_KEXEC_IMAGE && !(ima_appraise & IMA_APPRAISE_ENFORCE)
	    && security_locked_down(LOCKDOWN_KEXEC))
		return false;

	func = read_idmap[id] ?: FILE_CHECK;

	rcu_read_lock();
	ima_rules_tmp = rcu_dereference(ima_rules);
	list_for_each_entry_rcu(entry, ima_rules_tmp, list) {
		if (entry->action != APPRAISE)
			continue;

		/*
		 * A generic entry will match, but otherwise require that it
		 * match the func we're looking for
		 */
		if (entry->func && entry->func != func)
			continue;

		/*
		 * We require this to be a digital signature, not a raw IMA
		 * hash.
		 */
		if (entry->flags & IMA_DIGSIG_REQUIRED)
			found = true;

		/*
		 * We've found a rule that matches, so break now even if it
		 * didn't require a digital signature - a later rule that does
		 * won't override it, so would be a false positive.
		 */
		break;
	}