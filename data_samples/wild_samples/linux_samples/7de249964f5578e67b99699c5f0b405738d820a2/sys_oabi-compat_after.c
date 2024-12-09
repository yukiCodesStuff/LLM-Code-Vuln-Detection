{
	struct epoll_event *kbuf;
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
	set_fs(KERNEL_DS);
	ret = sys_epoll_wait(epfd, kbuf, maxevents, timeout);
	set_fs(fs);
	err = 0;
	for (i = 0; i < ret; i++) {
		__put_user_error(kbuf[i].events, &events->events, err);
		__put_user_error(kbuf[i].data,   &events->data,   err);
		events++;
	}
{
	struct sembuf *sops;
	struct timespec local_timeout;
	long err;
	int i;

	if (nsops < 1 || nsops > SEMOPM)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, tsops, sizeof(*tsops) * nsops))
		return -EFAULT;
	sops = kmalloc(sizeof(*sops) * nsops, GFP_KERNEL);
	if (!sops)
		return -ENOMEM;
	err = 0;
	for (i = 0; i < nsops; i++) {
		__get_user_error(sops[i].sem_num, &tsops->sem_num, err);
		__get_user_error(sops[i].sem_op,  &tsops->sem_op,  err);
		__get_user_error(sops[i].sem_flg, &tsops->sem_flg, err);
		tsops++;
	}