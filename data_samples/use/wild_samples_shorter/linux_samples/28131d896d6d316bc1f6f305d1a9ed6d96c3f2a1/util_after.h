
	if (fn)
		w->fn = fn;
	w->task = kthread_run(__mt76_worker_fn, w,
			      "mt76-%s %s", name, dev_name);

	if (IS_ERR(w->task)) {
		ret = PTR_ERR(w->task);
		w->task = NULL;
		return ret;
	}

	return 0;
}

static inline void mt76_worker_schedule(struct mt76_worker *w)