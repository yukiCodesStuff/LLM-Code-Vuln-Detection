	if (!bt->msg_data)
		goto err;

#ifdef CONFIG_BLK_DEBUG_FS
	/*
	 * When tracing whole make_request drivers (multiqueue) block devices,
	 * reuse the existing debugfs directory created by the block layer on
	 * init. For request-based block devices, all partitions block devices,
	 * and scsi-generic block devices we create a temporary new debugfs
	 * directory that will be removed once the trace ends.
	 */
	if (queue_is_mq(q) && bdev && bdev == bdev->bd_contains)
		dir = q->debugfs_dir;
	else
#endif
		bt->dir = dir = debugfs_create_dir(buts->name, blk_debugfs_root);

	bt->dev = dev;
	atomic_set(&bt->dropped, 0);

	ret = 0;
err:
	if (ret)
		blk_trace_free(bt);
	return ret;
}