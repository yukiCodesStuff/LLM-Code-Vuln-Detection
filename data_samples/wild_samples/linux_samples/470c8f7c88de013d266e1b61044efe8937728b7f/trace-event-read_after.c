	if (buf == NULL) {
		pr_debug("memory allocation failure\n");
		return -1;
	}

	ret = do_read(buf, size);
	if (ret < 0)
		goto out;

	ret = parse_event_file(pevent, buf, size, sys);
	if (ret < 0)
		pr_debug("error parsing event file.\n");
out:
	free(buf);
	return ret;
}

static int read_ftrace_files(struct tep_handle *pevent)
{
	unsigned long long size;
	int count;
	int i;
	int ret;

	count = read4(pevent);

	for (i = 0; i < count; i++) {
		size = read8(pevent);
		ret = read_ftrace_file(pevent, size);
		if (ret)
			return ret;
	}