		.aborted = aborted,
	};

	if (timer_pending(&mac->scan_timeout))
		del_timer_sync(&mac->scan_timeout);

	mutex_lock(&mac->mac_lock);

	if (mac->scan_req) {
		cfg80211_scan_done(mac->scan_req, &info);