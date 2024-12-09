	if (retval < 0)
		goto ret;

	if (fmt->flags & MISC_FMT_OPEN_FILE)
		interp_file = file_clone_open(fmt->interp_file);
	else
		interp_file = open_exec(fmt->interpreter);
	retval = PTR_ERR(interp_file);
	if (IS_ERR(interp_file))
		goto ret;
