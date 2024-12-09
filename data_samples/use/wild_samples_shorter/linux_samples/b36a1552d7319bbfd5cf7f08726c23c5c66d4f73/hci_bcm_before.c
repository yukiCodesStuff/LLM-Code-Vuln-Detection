
	bt_dev_dbg(hu->hdev, "hu %p", hu);

	bcm = kzalloc(sizeof(*bcm), GFP_KERNEL);
	if (!bcm)
		return -ENOMEM;
