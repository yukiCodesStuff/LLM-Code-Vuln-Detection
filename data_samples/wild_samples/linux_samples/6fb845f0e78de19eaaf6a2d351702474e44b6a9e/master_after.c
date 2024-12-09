{
	struct i3c_device_info info = { .dyn_addr = addr };
	struct i3c_dev_desc *newdev, *olddev;
	u8 old_dyn_addr = addr, expected_dyn_addr;
	struct i3c_ibi_setup ibireq = { };
	bool enable_ibi = false;
	int ret;

	if (!master)
		return -EINVAL;

	newdev = i3c_master_alloc_i3c_dev(master, &info);
	if (IS_ERR(newdev))
		return PTR_ERR(newdev);

	ret = i3c_master_attach_i3c_dev(master, newdev);
	if (ret)
		goto err_free_dev;

	ret = i3c_master_retrieve_dev_info(newdev);
	if (ret)
		goto err_detach_dev;

	olddev = i3c_master_search_i3c_dev_duplicate(newdev);
	if (olddev) {
		newdev->boardinfo = olddev->boardinfo;
		newdev->info.static_addr = olddev->info.static_addr;
		newdev->dev = olddev->dev;
		if (newdev->dev)
			newdev->dev->desc = newdev;

		/*
		 * We need to restore the IBI state too, so let's save the
		 * IBI information and try to restore them after olddev has
		 * been detached+released and its IBI has been stopped and
		 * the associated resources have been freed.
		 */
		mutex_lock(&olddev->ibi_lock);
		if (olddev->ibi) {
			ibireq.handler = olddev->ibi->handler;
			ibireq.max_payload_len = olddev->ibi->max_payload_len;
			ibireq.num_slots = olddev->ibi->num_slots;

			if (olddev->ibi->enabled) {
				enable_ibi = true;
				i3c_dev_disable_ibi_locked(olddev);
			}

			i3c_dev_free_ibi_locked(olddev);
		}
		mutex_unlock(&olddev->ibi_lock);

		old_dyn_addr = olddev->info.dyn_addr;

		i3c_master_detach_i3c_dev(olddev);
		i3c_master_free_i3c_dev(olddev);
	}