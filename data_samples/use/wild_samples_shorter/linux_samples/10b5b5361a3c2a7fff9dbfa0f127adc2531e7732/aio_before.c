
	/* everything turned out well, dispose of the aiocb. */
	kiocb_free(iocb);

	/*
	 * We have to order our ring_info tail store above and test
	 * of the wait list below outside the wait lock.  This is
	if (head == tail)
		goto out;

	while (ret < nr) {
		long avail;
		struct io_event *ev;
		struct page *page;
	flush_dcache_page(ctx->ring_pages[0]);

	pr_debug("%li  h%u t%u\n", ret, head, tail);

	put_reqs_available(ctx, ret);
out:
	mutex_unlock(&ctx->ring_lock);

	return ret;