{
	void *buf;
	size_t buf_size;
	size_t cmdline_len;
	int ret;

	cmdline_len = cmdline ? strlen(cmdline) : 0;
	buf_size = fdt_totalsize(initial_boot_params)
			+ cmdline_len + DTB_EXTRA_SPACE;

	for (;;) {
		buf = vmalloc(buf_size);
		if (!buf)