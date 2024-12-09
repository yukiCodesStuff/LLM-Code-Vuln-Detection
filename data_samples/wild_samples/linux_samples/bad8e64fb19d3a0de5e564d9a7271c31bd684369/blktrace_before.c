				      lockdep_is_held(&q->blk_trace_mutex))) {
		pr_warn("Concurrent blktraces are not allowed on %s\n",
			buts->name);
		return -EBUSY;
	}

	bt = kzalloc(sizeof(*bt), GFP_KERNEL);
	if (!bt)
		return -ENOMEM;

	ret = -ENOMEM;
	bt->sequence = alloc_percpu(unsigned long);
	if (!bt->sequence)
		goto err;

	bt->msg_data = __alloc_percpu(BLK_TN_MAX_MSG, __alignof__(char));
	if (!bt->msg_data)
		goto err;

	ret = -ENOENT;

	dir = debugfs_lookup(buts->name, blk_debugfs_root);
	if (!dir)
		bt->dir = dir = debugfs_create_dir(buts->name, blk_debugfs_root);

	bt->dev = dev;
	atomic_set(&bt->dropped, 0);
	INIT_LIST_HEAD(&bt->running_list);

	ret = -EIO;
	bt->dropped_file = debugfs_create_file("dropped", 0444, dir, bt,
					       &blk_dropped_fops);

	bt->msg_file = debugfs_create_file("msg", 0222, dir, bt, &blk_msg_fops);

	bt->rchan = relay_open("trace", dir, buts->buf_size,
				buts->buf_nr, &blk_relay_callbacks, bt);
	if (!bt->rchan)
		goto err;

	bt->act_mask = buts->act_mask;
	if (!bt->act_mask)
		bt->act_mask = (u16) -1;

	blk_trace_setup_lba(bt, bdev);

	/* overwrite with user settings */
	if (buts->start_lba)
		bt->start_lba = buts->start_lba;
	if (buts->end_lba)
		bt->end_lba = buts->end_lba;

	bt->pid = buts->pid;
	bt->trace_state = Blktrace_setup;

	rcu_assign_pointer(q->blk_trace, bt);
	get_probe_ref();

	ret = 0;
err:
	if (dir && !bt->dir)
		dput(dir);
	if (ret)
		blk_trace_free(bt);
	return ret;
}

static int __blk_trace_setup(struct request_queue *q, char *name, dev_t dev,
			     struct block_device *bdev, char __user *arg)
{
	struct blk_user_trace_setup buts;
	int ret;

	ret = copy_from_user(&buts, arg, sizeof(buts));
	if (ret)
		return -EFAULT;

	ret = do_blk_trace_setup(q, name, dev, bdev, &buts);
	if (ret)
		return ret;

	if (copy_to_user(arg, &buts, sizeof(buts))) {
		__blk_trace_remove(q);
		return -EFAULT;
	}
				      lockdep_is_held(&q->blk_trace_mutex))) {
		pr_warn("Concurrent blktraces are not allowed on %s\n",
			buts->name);
		return -EBUSY;
	}

	bt = kzalloc(sizeof(*bt), GFP_KERNEL);
	if (!bt)
		return -ENOMEM;

	ret = -ENOMEM;
	bt->sequence = alloc_percpu(unsigned long);
	if (!bt->sequence)
		goto err;

	bt->msg_data = __alloc_percpu(BLK_TN_MAX_MSG, __alignof__(char));
	if (!bt->msg_data)
		goto err;

	ret = -ENOENT;

	dir = debugfs_lookup(buts->name, blk_debugfs_root);
	if (!dir)
		bt->dir = dir = debugfs_create_dir(buts->name, blk_debugfs_root);

	bt->dev = dev;
	atomic_set(&bt->dropped, 0);
	INIT_LIST_HEAD(&bt->running_list);

	ret = -EIO;
	bt->dropped_file = debugfs_create_file("dropped", 0444, dir, bt,
					       &blk_dropped_fops);

	bt->msg_file = debugfs_create_file("msg", 0222, dir, bt, &blk_msg_fops);

	bt->rchan = relay_open("trace", dir, buts->buf_size,
				buts->buf_nr, &blk_relay_callbacks, bt);
	if (!bt->rchan)
		goto err;

	bt->act_mask = buts->act_mask;
	if (!bt->act_mask)
		bt->act_mask = (u16) -1;

	blk_trace_setup_lba(bt, bdev);

	/* overwrite with user settings */
	if (buts->start_lba)
		bt->start_lba = buts->start_lba;
	if (buts->end_lba)
		bt->end_lba = buts->end_lba;

	bt->pid = buts->pid;
	bt->trace_state = Blktrace_setup;

	rcu_assign_pointer(q->blk_trace, bt);
	get_probe_ref();

	ret = 0;
err:
	if (dir && !bt->dir)
		dput(dir);
	if (ret)
		blk_trace_free(bt);
	return ret;
}

static int __blk_trace_setup(struct request_queue *q, char *name, dev_t dev,
			     struct block_device *bdev, char __user *arg)
{
	struct blk_user_trace_setup buts;
	int ret;

	ret = copy_from_user(&buts, arg, sizeof(buts));
	if (ret)
		return -EFAULT;

	ret = do_blk_trace_setup(q, name, dev, bdev, &buts);
	if (ret)
		return ret;

	if (copy_to_user(arg, &buts, sizeof(buts))) {
		__blk_trace_remove(q);
		return -EFAULT;
	}