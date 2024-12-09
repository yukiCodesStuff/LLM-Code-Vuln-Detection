			      (void *)seg,
			      (int)len,
			      &transfer,
			      HZ * 5);

	if (status < 0) {
		rsi_dbg(ERR_ZONE,
			"Card write failed with error code :%10d\n", status);