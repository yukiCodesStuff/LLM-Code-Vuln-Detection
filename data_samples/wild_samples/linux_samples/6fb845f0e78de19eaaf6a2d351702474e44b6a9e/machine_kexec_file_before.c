{
	void *buf;
	size_t buf_size;
	int ret;

	buf_size = fdt_totalsize(initial_boot_params)
			+ strlen(cmdline) + DTB_EXTRA_SPACE;

	for (;;) {
		buf = vmalloc(buf_size);
		if (!buf)
			return -ENOMEM;

		/* duplicate a device tree blob */
		ret = fdt_open_into(initial_boot_params, buf, buf_size);
		if (ret)
			return -EINVAL;

		ret = setup_dtb(image, initrd_load_addr, initrd_len,
				cmdline, buf);
		if (ret) {
			vfree(buf);
			if (ret == -ENOMEM) {
				/* unlikely, but just in case */
				buf_size += DTB_EXTRA_SPACE;
				continue;
			} else {
				return ret;
			}
		}

		/* trim it */
		fdt_pack(buf);
		*dtb = buf;

		return 0;
	}
}