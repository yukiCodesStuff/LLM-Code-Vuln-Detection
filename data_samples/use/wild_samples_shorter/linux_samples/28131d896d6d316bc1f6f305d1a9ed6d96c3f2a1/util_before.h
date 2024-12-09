
	if (fn)
		w->fn = fn;
	w->task = kthread_create(__mt76_worker_fn, w, "mt76-%s %s",
				 name, dev_name);

	ret = PTR_ERR_OR_ZERO(w->task);
	if (ret) {
		w->task = NULL;
		return ret;
	}

	wake_up_process(w->task);

	return 0;
}

static inline void mt76_worker_schedule(struct mt76_worker *w)