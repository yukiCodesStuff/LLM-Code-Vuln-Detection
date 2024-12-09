	if (cbuff == NULL)
		goto err_exit;
	slcomp = slhc_init(16, 16);
	if (IS_ERR(slcomp))
		goto err_exit;
#endif
	spin_lock_bh(&sl->lock);
	if (sl->tty == NULL) {