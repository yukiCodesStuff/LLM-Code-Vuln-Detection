	mm_segment_t fs;
	long ret, err, i;

	if (maxevents <= 0 || maxevents > (INT_MAX/sizeof(struct epoll_event)))
		return -EINVAL;
	kbuf = kmalloc(sizeof(*kbuf) * maxevents, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;
	fs = get_fs();

	if (nsops < 1 || nsops > SEMOPM)
		return -EINVAL;
	sops = kmalloc(sizeof(*sops) * nsops, GFP_KERNEL);
	if (!sops)
		return -ENOMEM;
	err = 0;