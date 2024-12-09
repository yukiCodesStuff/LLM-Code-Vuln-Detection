{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_aggregate *new_agg = qfq_find_agg(q, lmax, weight);

	if (new_agg == NULL) { /* create new aggregate */
		new_agg = kzalloc(sizeof(*new_agg), GFP_ATOMIC);
		if (new_agg == NULL)
			return -ENOBUFS;
		qfq_init_agg(q, new_agg, lmax, weight);
	}