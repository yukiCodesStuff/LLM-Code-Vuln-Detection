{
	struct mm_struct *mm = vmf->vma->vm_mm;
	struct userfaultfd_ctx *ctx;
	struct userfaultfd_wait_queue uwq;
	vm_fault_t ret = VM_FAULT_SIGBUS;
	bool must_wait;
	long blocking_state;

	/*
	 * We don't do userfault handling for the final child pid update.
	 *
	 * We also don't do userfault handling during
	 * coredumping. hugetlbfs has the special
	 * follow_hugetlb_page() to skip missing pages in the
	 * FOLL_DUMP case, anon memory also checks for FOLL_DUMP with
	 * the no_page_table() helper in follow_page_mask(), but the
	 * shmem_vm_ops->fault method is invoked even during
	 * coredumping without mmap_lock and it ends up here.
	 */
	if (current->flags & (PF_EXITING|PF_DUMPCORE))
		goto out;

	/*
	 * Coredumping runs without mmap_lock so we can only check that
	 * the mmap_lock is held, if PF_DUMPCORE was not set.
	 */
	mmap_assert_locked(mm);

	ctx = vmf->vma->vm_userfaultfd_ctx.ctx;
	if (!ctx)
		goto out;

	BUG_ON(ctx->mm != mm);

	VM_BUG_ON(reason & ~(VM_UFFD_MISSING|VM_UFFD_WP));
	VM_BUG_ON(!(reason & VM_UFFD_MISSING) ^ !!(reason & VM_UFFD_WP));

	if (ctx->features & UFFD_FEATURE_SIGBUS)
		goto out;
	if ((vmf->flags & FAULT_FLAG_USER) == 0 &&
	    ctx->flags & UFFD_USER_MODE_ONLY) {
		printk_once(KERN_WARNING "uffd: Set unprivileged_userfaultfd "
			"sysctl knob to 1 if kernel faults must be handled "
			"without obtaining CAP_SYS_PTRACE capability\n");
		goto out;
	}
{
	struct userfaultfd_ctx *ctx;
	int fd;

	if (!sysctl_unprivileged_userfaultfd && !capable(CAP_SYS_PTRACE))
		return -EPERM;

	BUG_ON(!current->mm);

	/* Check the UFFD_* constants for consistency.  */
	BUILD_BUG_ON(UFFD_USER_MODE_ONLY & UFFD_SHARED_FCNTL_FLAGS);
	BUILD_BUG_ON(UFFD_CLOEXEC != O_CLOEXEC);
	BUILD_BUG_ON(UFFD_NONBLOCK != O_NONBLOCK);

	if (flags & ~(UFFD_SHARED_FCNTL_FLAGS | UFFD_USER_MODE_ONLY))
		return -EINVAL;

	ctx = kmem_cache_alloc(userfaultfd_ctx_cachep, GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	refcount_set(&ctx->refcount, 1);
	ctx->flags = flags;
	ctx->features = 0;
	ctx->state = UFFD_STATE_WAIT_API;
	ctx->released = false;
	ctx->mmap_changing = false;
	ctx->mm = current->mm;
	/* prevent the mm struct to be freed */
	mmgrab(ctx->mm);

	fd = anon_inode_getfd("[userfaultfd]", &userfaultfd_fops, ctx,
			      O_RDWR | (flags & UFFD_SHARED_FCNTL_FLAGS));
	if (fd < 0) {
		mmdrop(ctx->mm);
		kmem_cache_free(userfaultfd_ctx_cachep, ctx);
	}