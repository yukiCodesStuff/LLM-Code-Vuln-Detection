		goto out_put_disk;

	q = vblk->disk->queue = blk_mq_init_queue(&vblk->tag_set);
	if (!q) {
		err = -ENOMEM;
		goto out_free_tags;
	}
