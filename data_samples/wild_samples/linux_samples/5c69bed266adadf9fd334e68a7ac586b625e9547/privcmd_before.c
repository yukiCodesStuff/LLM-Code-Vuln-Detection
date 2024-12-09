{
	struct privcmd_mmap mmapcmd;
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma;
	int rc;
	LIST_HEAD(pagelist);
	struct mmap_mfn_state state;

	if (!xen_initial_domain())
		return -EPERM;

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
	/* An array for individual errors */
	int *err;

	/* User-space mfn array to store errors in the second pass for V1. */
	xen_pfn_t __user *user_mfn;
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
	*(st->err++) = ret;

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
	*(st->err++) = ret;

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
		else {
			/* Record that at least one error has happened. */
			if (st->global_error == 0)
				st->global_error = 1;
		}
	}
	st->va += PAGE_SIZE;

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
 * Returns: 0 if success, otherwise -errno
 */
static int alloc_empty_pages(struct vm_area_struct *vma, int numpgs)
{
	int rc;
	struct page **pages;

	pages = kcalloc(numpgs, sizeof(pages[0]), GFP_KERNEL);
	if (pages == NULL)
		return -ENOMEM;

	rc = alloc_xenballooned_pages(numpgs, pages, 0);
	if (rc != 0) {
		pr_warn("%s Could not alloc %d pfns rc:%d\n", __func__,
			numpgs, rc);
		kfree(pages);
		return -ENOMEM;
	}
{
	int ret;
	struct privcmd_mmapbatch_v2 m;
	struct mm_struct *mm = current->mm;
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

	err_array = kcalloc(m.num, sizeof(int), GFP_KERNEL);
	if (err_array == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	down_write(&mm->mmap_sem);

	vma = find_vma(mm, m.addr);
	if (!vma ||
	    vma->vm_ops != &privcmd_vm_ops ||
	    (m.addr != vma->vm_start) ||
	    ((m.addr + (nr_pages << PAGE_SHIFT)) != vma->vm_end) ||
	    !privcmd_enforce_singleshot_mapping(vma)) {
		up_write(&mm->mmap_sem);
		ret = -EINVAL;
		goto out;
	}
	if (xen_feature(XENFEAT_auto_translated_physmap)) {
		ret = alloc_empty_pages(vma, m.num);
		if (ret < 0) {
			up_write(&mm->mmap_sem);
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

static long privcmd_ioctl(struct file *file,
			  unsigned int cmd, unsigned long data)
{
	int ret = -ENOSYS;
	void __user *udata = (void __user *) data;

	switch (cmd) {
	case IOCTL_PRIVCMD_HYPERCALL:
		ret = privcmd_ioctl_hypercall(udata);
		break;

	case IOCTL_PRIVCMD_MMAP:
		ret = privcmd_ioctl_mmap(udata);
		break;

	case IOCTL_PRIVCMD_MMAPBATCH:
		ret = privcmd_ioctl_mmap_batch(udata, 1);
		break;

	case IOCTL_PRIVCMD_MMAPBATCH_V2:
		ret = privcmd_ioctl_mmap_batch(udata, 2);
		break;

	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static void privcmd_close(struct vm_area_struct *vma)
{
	struct page **pages = vma->vm_private_data;
	int numpgs = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;

	if (!xen_feature(XENFEAT_auto_translated_physmap || !numpgs || !pages))
		return;

	xen_unmap_domain_mfn_range(vma, numpgs, pages);
	free_xenballooned_pages(numpgs, pages);
	kfree(pages);
}

static int privcmd_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	printk(KERN_DEBUG "privcmd_fault: vma=%p %lx-%lx, pgoff=%lx, uv=%p\n",
	       vma, vma->vm_start, vma->vm_end,
	       vmf->pgoff, vmf->virtual_address);

	return VM_FAULT_SIGBUS;
}

static struct vm_operations_struct privcmd_vm_ops = {
	.close = privcmd_close,
	.fault = privcmd_fault
};

static int privcmd_mmap(struct file *file, struct vm_area_struct *vma)
{
	/* DONTCOPY is essential for Xen because copy_page_range doesn't know
	 * how to recreate these mappings */
	vma->vm_flags |= VM_IO | VM_PFNMAP | VM_DONTCOPY |
			 VM_DONTEXPAND | VM_DONTDUMP;
	vma->vm_ops = &privcmd_vm_ops;
	vma->vm_private_data = NULL;

	return 0;
}

static int privcmd_enforce_singleshot_mapping(struct vm_area_struct *vma)
{
	return !cmpxchg(&vma->vm_private_data, NULL, PRIV_VMA_LOCKED);
}

const struct file_operations xen_privcmd_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = privcmd_ioctl,
	.mmap = privcmd_mmap,
};
EXPORT_SYMBOL_GPL(xen_privcmd_fops);

static struct miscdevice privcmd_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "xen/privcmd",
	.fops = &xen_privcmd_fops,
};

static int __init privcmd_init(void)
{
	int err;

	if (!xen_domain())
		return -ENODEV;

	err = misc_register(&privcmd_dev);
	if (err != 0) {
		printk(KERN_ERR "Could not register Xen privcmd device\n");
		return err;
	}
	return 0;
}

static void __exit privcmd_exit(void)
{
	misc_deregister(&privcmd_dev);
}

module_init(privcmd_init);
module_exit(privcmd_exit);