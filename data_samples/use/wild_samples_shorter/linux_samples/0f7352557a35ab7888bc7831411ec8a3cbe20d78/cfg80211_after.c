	scan_request = cfg->scan_request;
	cfg->scan_request = NULL;

	timer_delete_sync(&cfg->escan_timeout);

	if (fw_abort) {
		/* Do a scan abort to stop the driver's scan engine */
		brcmf_dbg(SCAN, "ABORT scan in firmware\n");
	brcmf_btcoex_detach(cfg);
	wiphy_unregister(cfg->wiphy);
	wl_deinit_priv(cfg);
	cancel_work_sync(&cfg->escan_timeout_work);
	brcmf_free_wiphy(cfg->wiphy);
	kfree(cfg);
}