out:
	if (err) {
		m->tcf_qstats.overlimits++;
		/* should we be asking for packet to be dropped?
		 * may make sense for redirect case only
		 */
		retval = TC_ACT_SHOT;
	} else {
		retval = m->tcf_action;
	}
	spin_unlock(&m->tcf_lock);

	return retval;
}