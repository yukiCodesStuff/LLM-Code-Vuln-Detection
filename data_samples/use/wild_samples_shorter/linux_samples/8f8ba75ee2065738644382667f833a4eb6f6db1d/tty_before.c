
	size = sizeof(*dl) + dev_num * sizeof(*di);

	dl = kmalloc(size, GFP_KERNEL);
	if (!dl)
		return -ENOMEM;

	di = dl->dev_info;