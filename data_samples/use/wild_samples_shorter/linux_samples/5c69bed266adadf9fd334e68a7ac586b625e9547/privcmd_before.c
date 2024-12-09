	LIST_HEAD(pagelist);
	struct mmap_mfn_state state;

	if (!xen_initial_domain())
		return -EPERM;

	/* We only support privcmd_ioctl_mmap_batch for auto translated. */
	if (xen_feature(XENFEAT_auto_translated_physmap))
		return -ENOSYS;

	 *      -ENOENT if at least 1 -ENOENT has happened.
	 */
	int global_error;
	/* An array for individual errors */
	int *err;

	/* User-space mfn array to store errors in the second pass for V1. */
	xen_pfn_t __user *user_mfn;
};

/* auto translated dom0 note: if domU being created is PV, then mfn is
 * mfn(addr on bus). If it's auto xlated, then mfn is pfn (input to HAP).
					 &cur_page);

	/* Store error code for second pass. */
	*(st->err++) = ret;

	/* And see if it affects the global_error. */
	if (ret < 0) {
		if (ret == -ENOENT)
	return 0;
}

static int mmap_return_errors_v1(void *data, void *state)
{
	xen_pfn_t *mfnp = data;
	struct mmap_batch_state *st = state;
	int err = *(st->err++);

	/*
	 * V1 encodes the error codes in the 32bit top nibble of the
	 * mfn (with its known limitations vis-a-vis 64 bit callers).
	 */
	*mfnp |= (err == -ENOENT) ?
				PRIVCMD_MMAPBATCH_PAGED_ERROR :
				PRIVCMD_MMAPBATCH_MFN_ERROR;
	return __put_user(*mfnp, st->user_mfn++);
}

/* Allocate pfns that are then mapped with gmfns from foreign domid. Update
 * the vma with the page info to use later.
	struct vm_area_struct *vma;
	unsigned long nr_pages;
	LIST_HEAD(pagelist);
	int *err_array = NULL;
	struct mmap_batch_state state;

	if (!xen_initial_domain())
		return -EPERM;

	switch (version) {
	case 1:
		if (copy_from_user(&m, udata, sizeof(struct privcmd_mmapbatch)))
			return -EFAULT;
		goto out;
	}

	err_array = kcalloc(m.num, sizeof(int), GFP_KERNEL);
	if (err_array == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	down_write(&mm->mmap_sem);

	state.va            = m.addr;
	state.index         = 0;
	state.global_error  = 0;
	state.err           = err_array;

	/* mmap_batch_fn guarantees ret == 0 */
	BUG_ON(traverse_pages(m.num, sizeof(xen_pfn_t),
			     &pagelist, mmap_batch_fn, &state));

	up_write(&mm->mmap_sem);

	if (version == 1) {
		if (state.global_error) {
			/* Write back errors in second pass. */
			state.user_mfn = (xen_pfn_t *)m.arr;
			state.err      = err_array;
			ret = traverse_pages(m.num, sizeof(xen_pfn_t),
					     &pagelist, mmap_return_errors_v1, &state);
		} else
			ret = 0;

	} else if (version == 2) {
		ret = __copy_to_user(m.err, err_array, m.num * sizeof(int));
		if (ret)
			ret = -EFAULT;
	}

	/* If we have not had any EFAULT-like global errors then set the global
	 * error to -ENOENT if necessary. */
	if ((ret == 0) && (state.global_error == -ENOENT))
		ret = -ENOENT;

out:
	kfree(err_array);
	free_page_list(&pagelist);

	return ret;
}