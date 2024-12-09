	*iovcount = seg;
	if (unlikely(log))
		*log_num = nlogs;
	return headcount;
err:
	vhost_discard_vq_desc(vq, headcount);
	return r;
		/* On error, stop handling until the next kick. */
		if (unlikely(headcount < 0))
			break;
		/* OK, now we need to know about added descriptors. */
		if (!headcount) {
			if (unlikely(vhost_enable_notify(&net->dev, vq))) {
				/* They have slipped one in as we were