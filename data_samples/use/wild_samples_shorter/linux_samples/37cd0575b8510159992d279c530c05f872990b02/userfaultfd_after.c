
	if (ctx->features & UFFD_FEATURE_SIGBUS)
		goto out;
	if ((vmf->flags & FAULT_FLAG_USER) == 0 &&
	    ctx->flags & UFFD_USER_MODE_ONLY) {
		printk_once(KERN_WARNING "uffd: Set unprivileged_userfaultfd "
			"sysctl knob to 1 if kernel faults must be handled "
			"without obtaining CAP_SYS_PTRACE capability\n");
		goto out;
	}

	/*
	 * If it's already released don't get it. This avoids to loop
	 * in __get_user_pages if userfaultfd_release waits on the
	BUG_ON(!current->mm);

	/* Check the UFFD_* constants for consistency.  */
	BUILD_BUG_ON(UFFD_USER_MODE_ONLY & UFFD_SHARED_FCNTL_FLAGS);
	BUILD_BUG_ON(UFFD_CLOEXEC != O_CLOEXEC);
	BUILD_BUG_ON(UFFD_NONBLOCK != O_NONBLOCK);

	if (flags & ~(UFFD_SHARED_FCNTL_FLAGS | UFFD_USER_MODE_ONLY))
		return -EINVAL;

	ctx = kmem_cache_alloc(userfaultfd_ctx_cachep, GFP_KERNEL);
	if (!ctx)