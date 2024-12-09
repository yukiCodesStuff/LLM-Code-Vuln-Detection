	mm_segment_t fs;
	long ret, err, i;

	if (maxevents <= 0 ||
			maxevents > (INT_MAX/sizeof(*kbuf)) ||
			maxevents > (INT_MAX/sizeof(*events)))
		return -EINVAL;
	if (!access_ok(VERIFY_WRITE, events, sizeof(*events) * maxevents))
		return -EFAULT;
	kbuf = kmalloc(sizeof(*kbuf) * maxevents, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;
	fs = get_fs();

	if (nsops < 1 || nsops > SEMOPM)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, tsops, sizeof(*tsops) * nsops))
		return -EFAULT;
	sops = kmalloc(sizeof(*sops) * nsops, GFP_KERNEL);
	if (!sops)
		return -ENOMEM;
	err = 0;