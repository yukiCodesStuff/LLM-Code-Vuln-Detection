{
	const char *dev_name = wiphy_name(hw->wiphy);
	int ret;

	if (fn)
		w->fn = fn;
	w->task = kthread_create(__mt76_worker_fn, w, "mt76-%s %s",
				 name, dev_name);

	ret = PTR_ERR_OR_ZERO(w->task);
	if (ret) {
		w->task = NULL;
		return ret;
	}