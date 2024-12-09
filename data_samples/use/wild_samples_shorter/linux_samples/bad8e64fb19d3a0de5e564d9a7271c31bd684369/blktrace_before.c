	if (!bt->msg_data)
		goto err;

	ret = -ENOENT;

	dir = debugfs_lookup(buts->name, blk_debugfs_root);
	if (!dir)
		bt->dir = dir = debugfs_create_dir(buts->name, blk_debugfs_root);

	bt->dev = dev;
	atomic_set(&bt->dropped, 0);

	ret = 0;
err:
	if (dir && !bt->dir)
		dput(dir);
	if (ret)
		blk_trace_free(bt);
	return ret;
}