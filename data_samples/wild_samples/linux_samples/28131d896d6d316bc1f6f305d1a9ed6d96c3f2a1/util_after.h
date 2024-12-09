{
	const char *dev_name = wiphy_name(hw->wiphy);
	int ret;

	if (fn)
		w->fn = fn;
	w->task = kthread_run(__mt76_worker_fn, w,
			      "mt76-%s %s", name, dev_name);

	if (IS_ERR(w->task)) {
		ret = PTR_ERR(w->task);
		w->task = NULL;
		return ret;
	}