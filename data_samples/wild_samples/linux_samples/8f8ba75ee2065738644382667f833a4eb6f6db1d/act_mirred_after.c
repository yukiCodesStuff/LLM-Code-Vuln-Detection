	if (err) {
		m->tcf_qstats.overlimits++;
		if (m->tcfm_eaction != TCA_EGRESS_MIRROR)
			retval = TC_ACT_SHOT;
		else
			retval = m->tcf_action;
	} else
		retval = m->tcf_action;
	spin_unlock(&m->tcf_lock);

	return retval;
}