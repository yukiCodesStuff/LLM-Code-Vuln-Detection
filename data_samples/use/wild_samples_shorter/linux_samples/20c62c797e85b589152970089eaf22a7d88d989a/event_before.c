		return -EINVAL;
	}

	if (timer_pending(&mac->scan_timeout))
		del_timer_sync(&mac->scan_timeout);
	qtnf_scan_done(mac, le32_to_cpu(status->flags) & QLINK_SCAN_ABORTED);

	return 0;
}