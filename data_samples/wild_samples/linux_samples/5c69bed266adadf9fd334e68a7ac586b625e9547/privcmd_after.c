{
	struct privcmd_mmap mmapcmd;
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma;
	int rc;
	LIST_HEAD(pagelist);
	struct mmap_mfn_state state;

	/* We only support privcmd_ioctl_mmap_batch for auto translated. */
	if (xen_feature(XENFEAT_auto_translated_physmap))
		return -ENOSYS;

	if (copy_from_user(&mmapcmd, udata, sizeof(mmapcmd)))
		return -EFAULT;

	rc = gather_array(&pagelist,
			  mmapcmd.num, sizeof(struct privcmd_mmap_entry),
			  mmapcmd.entry);

	if (rc || list_empty(&pagelist))
		goto out;

	down_write(&mm->mmap_sem);

	{
		struct page *page = list_first_entry(&pagelist,
						     struct page, lru);
		struct privcmd_mmap_entry *msg = page_address(page);

		vma = find_vma(mm, msg->va);
		rc = -EINVAL;

		if (!vma || (msg->va != vma->vm_start) ||
		    !privcmd_enforce_singleshot_mapping(vma))
			goto out_up;
	}
struct mmap_batch_state {
	domid_t domain;
	unsigned long va;
	struct vm_area_struct *vma;
	int index;
	/* A tristate:
	 *      0 for no errors
	 *      1 if at least one error has happened (and no
	 *          -ENOENT errors have happened)
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
 */
static int mmap_batch_fn(void *data, void *state)
{
	xen_pfn_t *mfnp = data;
	struct mmap_batch_state *st = state;
	struct vm_area_struct *vma = st->vma;
	struct page **pages = vma->vm_private_data;
	struct page *cur_page = NULL;
	int ret;

	if (xen_feature(XENFEAT_auto_translated_physmap))
		cur_page = pages[st->index++];

	ret = xen_remap_domain_mfn_range(st->vma, st->va & PAGE_MASK, *mfnp, 1,
					 st->vma->vm_page_prot, st->domain,
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
			st->global_error = -ENOENT;
		else {
			/* Record that at least one error has happened. */
			if (st->global_error == 0)
				st->global_error = 1;
		}
	}
	st->va += PAGE_SIZE;

	return 0;
}
{
	xen_pfn_t *mfnp = data;
	struct mmap_batch_state *st = state;
	struct vm_area_struct *vma = st->vma;
	struct page **pages = vma->vm_private_data;
	struct page *cur_page = NULL;
	int ret;

	if (xen_feature(XENFEAT_auto_translated_physmap))
		cur_page = pages[st->index++];

	ret = xen_remap_domain_mfn_range(st->vma, st->va & PAGE_MASK, *mfnp, 1,
					 st->vma->vm_page_prot, st->domain,
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
		else {
			/* Record that at least one error has happened. */
			if (st->global_error == 0)
				st->global_error = 1;
		}
	}
	st->va += PAGE_SIZE;

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
{
	int ret;
	struct privcmd_mmapbatch_v2 m;
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma;
	unsigned long nr_pages;
	LIST_HEAD(pagelist);
	struct mmap_batch_state state;

	switch (version) {
	case 1:
		if (copy_from_user(&m, udata, sizeof(struct privcmd_mmapbatch)))
			return -EFAULT;
		/* Returns per-frame error in m.arr. */
		m.err = NULL;
		if (!access_ok(VERIFY_WRITE, m.arr, m.num * sizeof(*m.arr)))
			return -EFAULT;
		break;
	case 2:
		if (copy_from_user(&m, udata, sizeof(struct privcmd_mmapbatch_v2)))
			return -EFAULT;
		/* Returns per-frame error code in m.err. */
		if (!access_ok(VERIFY_WRITE, m.err, m.num * (sizeof(*m.err))))
			return -EFAULT;
		break;
	default:
		return -EINVAL;
	}
	if (list_empty(&pagelist)) {
		ret = -EINVAL;
		goto out;
	}

	if (version == 2) {
		/* Zero error array now to only copy back actual errors. */
		if (clear_user(m.err, sizeof(int) * m.num)) {
			ret = -EFAULT;
			goto out;
		}
		if (ret < 0) {
			up_write(&mm->mmap_sem);
			goto out;
		}
	}

	state.domain        = m.dom;
	state.vma           = vma;
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