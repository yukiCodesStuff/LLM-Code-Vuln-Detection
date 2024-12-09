{
	void *buf;
	size_t buf_size;
	int ret;

	buf_size = fdt_totalsize(initial_boot_params)
			+ strlen(cmdline) + DTB_EXTRA_SPACE;

	for (;;) {
		buf = vmalloc(buf_size);
		if (!buf)