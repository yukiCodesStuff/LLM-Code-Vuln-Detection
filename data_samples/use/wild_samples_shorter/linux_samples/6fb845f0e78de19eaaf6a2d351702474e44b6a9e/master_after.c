
	ret = i3c_master_retrieve_dev_info(newdev);
	if (ret)
		goto err_detach_dev;

	olddev = i3c_master_search_i3c_dev_duplicate(newdev);
	if (olddev) {
		newdev->boardinfo = olddev->boardinfo;