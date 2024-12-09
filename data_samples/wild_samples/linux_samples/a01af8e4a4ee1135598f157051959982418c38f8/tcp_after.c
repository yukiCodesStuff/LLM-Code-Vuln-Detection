	switch (optname) {
	case TCP_MAXSEG:
		/* Values greater than interface MTU won't take effect. However
		 * at the point when this call is done we typically don't yet
		 * know which interface is going to be used */
		if (val < TCP_MIN_MSS || val > MAX_TCP_WINDOW) {
			err = -EINVAL;
			break;
		}