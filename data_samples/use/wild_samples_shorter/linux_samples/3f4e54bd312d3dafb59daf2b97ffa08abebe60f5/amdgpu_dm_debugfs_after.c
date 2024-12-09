	if (!wr_buf)
		return -ENOSPC;

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					   (long *)param, buf,
					   max_param_num,
					   &param_nums)) {
		kfree(wr_buf);
	if (!wr_buf)
		return -ENOSPC;

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					   (long *)param, buf,
					   max_param_num,
					   &param_nums)) {
		kfree(wr_buf);
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					   &param, buf,
					   max_param_num,
					   &param_nums)) {
		kfree(wr_buf);
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
						(long *)param, buf,
						max_param_num,
						&param_nums)) {
		kfree(wr_buf);
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					    (long *)param, buf,
					    max_param_num,
					    &param_nums)) {
		kfree(wr_buf);
		return -ENOSPC;
	}

	if (parse_write_buffer_into_params(wr_buf, wr_buf_size,
					   (long *)param, buf,
					   max_param_num,
					   &param_nums)) {
		kfree(wr_buf);