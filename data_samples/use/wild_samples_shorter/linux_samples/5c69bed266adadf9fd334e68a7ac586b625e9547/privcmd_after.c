	LIST_HEAD(pagelist);
	struct mmap_mfn_state state;

	/* We only support privcmd_ioctl_mmap_batch for auto translated. */
	if (xen_feature(XENFEAT_auto_translated_physmap))
		return -ENOSYS;

	 *      -ENOENT if at least 1 -ENOENT has happened.
	 */
	int global_error;
	int version;

	/* User-space mfn array to store errors in the second pass for V1. */
	xen_pfn_t __user *user_mfn;
	/* User-space int array to store errors in the second pass for V2. */
	int __user *user_err;
};

/* auto translated dom0 note: if domU being created is PV, then mfn is
 * mfn(addr on bus). If it's auto xlated, then mfn is pfn (input to HAP).
					 &cur_page);

	/* Store error code for second pass. */
	if (st->version == 1) {
		if (ret < 0) {
			/*
			 * V1 encodes the error codes in the 32bit top nibble of the
			 * mfn (with its known limitations vis-a-vis 64 bit callers).
			 */
			*mfnp |= (ret == -ENOENT) ?
						PRIVCMD_MMAPBATCH_PAGED_ERROR :
						PRIVCMD_MMAPBATCH_MFN_ERROR;
		}
	} else { /* st->version == 2 */
		*((int *) mfnp) = ret;
	}

	/* And see if it affects the global_error. */
	if (ret < 0) {
		if (ret == -ENOENT)
	return 0;
}

static int mmap_return_errors(void *data, void *state)
{
	struct mmap_batch_state *st = state;

	if (st->version == 1) {
		xen_pfn_t mfnp = *((xen_pfn_t *) data);
		if (mfnp & PRIVCMD_MMAPBATCH_MFN_ERROR)
			return __put_user(mfnp, st->user_mfn++);
		else
			st->user_mfn++;
	} else { /* st->version == 2 */
		int err = *((int *) data);
		if (err)
			return __put_user(err, st->user_err++);
		else
			st->user_err++;
	}

	return 0;
}

/* Allocate pfns that are then mapped with gmfns from foreign domid. Update
 * the vma with the page info to use later.
	struct vm_area_struct *vma;
	unsigned long nr_pages;
	LIST_HEAD(pagelist);
	struct mmap_batch_state state;

	switch (version) {
	case 1:
		if (copy_from_user(&m, udata, sizeof(struct privcmd_mmapbatch)))
			return -EFAULT;
		goto out;
	}

	if (version == 2) {
		/* Zero error array now to only copy back actual errors. */
		if (clear_user(m.err, sizeof(int) * m.num)) {
			ret = -EFAULT;
			goto out;
		}
	}

	down_write(&mm->mmap_sem);

	state.va            = m.addr;
	state.index         = 0;
	state.global_error  = 0;
	state.version       = version;

	/* mmap_batch_fn guarantees ret == 0 */
	BUG_ON(traverse_pages(m.num, sizeof(xen_pfn_t),
			     &pagelist, mmap_batch_fn, &state));

	up_write(&mm->mmap_sem);

	if (state.global_error) {
		/* Write back errors in second pass. */
		state.user_mfn = (xen_pfn_t *)m.arr;
		state.user_err = m.err;
		ret = traverse_pages(m.num, sizeof(xen_pfn_t),
							 &pagelist, mmap_return_errors, &state);
	} else
		ret = 0;

	/* If we have not had any EFAULT-like global errors then set the global
	 * error to -ENOENT if necessary. */
	if ((ret == 0) && (state.global_error == -ENOENT))
		ret = -ENOENT;

out:
	free_page_list(&pagelist);

	return ret;
}