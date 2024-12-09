			      (void *)seg,
			      (int)len,
			      &transfer,
			      USB_CTRL_SET_TIMEOUT);

	if (status < 0) {
		rsi_dbg(ERR_ZONE,
			"Card write failed with error code :%10d\n", status);