	}

	ret = do_read(buf, size);
	if (ret < 0)
		goto out;

	ret = parse_event_file(pevent, buf, size, sys);
	if (ret < 0)
		pr_debug("error parsing event file.\n");