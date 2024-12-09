{
	struct ib_ucontext *ctx = per_mm->context;
	struct ib_umem_odp *odp_data;
	struct ib_umem *umem;
	int pages = size >> PAGE_SHIFT;
	int ret;

	odp_data = kzalloc(sizeof(*odp_data), GFP_KERNEL);
	if (!odp_data)
		return ERR_PTR(-ENOMEM);
	umem = &odp_data->umem;
	umem->context    = ctx;
	umem->length     = size;
	umem->address    = addr;
	umem->page_shift = PAGE_SHIFT;
	umem->writable   = 1;
	umem->is_odp = 1;
	odp_data->per_mm = per_mm;

	mutex_init(&odp_data->umem_mutex);
	init_completion(&odp_data->notifier_completion);

	odp_data->page_list =
		vzalloc(array_size(pages, sizeof(*odp_data->page_list)));
	if (!odp_data->page_list) {
		ret = -ENOMEM;
		goto out_odp_data;
	}
	if (!odp_data->dma_list) {
		ret = -ENOMEM;
		goto out_page_list;
	}

	/*
	 * Caller must ensure that the umem_odp that the per_mm came from
	 * cannot be freed during the call to ib_alloc_odp_umem.
	 */
	mutex_lock(&ctx->per_mm_list_lock);
	per_mm->odp_mrs_count++;
	mutex_unlock(&ctx->per_mm_list_lock);
	add_umem_to_per_mm(odp_data);

	return odp_data;

out_page_list:
	vfree(odp_data->page_list);
out_odp_data:
	kfree(odp_data);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL(ib_alloc_odp_umem);

int ib_umem_odp_get(struct ib_umem_odp *umem_odp, int access)
{
	struct ib_umem *umem = &umem_odp->umem;
	/*
	 * NOTE: This must called in a process context where umem->owning_mm
	 * == current->mm
	 */
	struct mm_struct *mm = umem->owning_mm;
	int ret_val;

	if (access & IB_ACCESS_HUGETLB) {
		struct vm_area_struct *vma;
		struct hstate *h;

		down_read(&mm->mmap_sem);
		vma = find_vma(mm, ib_umem_start(umem));
		if (!vma || !is_vm_hugetlb_page(vma)) {
			up_read(&mm->mmap_sem);
			return -EINVAL;
		}
		h = hstate_vma(vma);
		umem->page_shift = huge_page_shift(h);
		up_read(&mm->mmap_sem);
		umem->hugetlb = 1;
	} else {
		umem->hugetlb = 0;
	}

	mutex_init(&umem_odp->umem_mutex);

	init_completion(&umem_odp->notifier_completion);

	if (ib_umem_num_pages(umem)) {
		umem_odp->page_list =
			vzalloc(array_size(sizeof(*umem_odp->page_list),
					   ib_umem_num_pages(umem)));
		if (!umem_odp->page_list)
			return -ENOMEM;

		umem_odp->dma_list =
			vzalloc(array_size(sizeof(*umem_odp->dma_list),
					   ib_umem_num_pages(umem)));
		if (!umem_odp->dma_list) {
			ret_val = -ENOMEM;
			goto out_page_list;
		}
	}

	ret_val = get_per_mm(umem_odp);
	if (ret_val)
		goto out_dma_list;
	add_umem_to_per_mm(umem_odp);

	return 0;

out_dma_list:
	vfree(umem_odp->dma_list);
out_page_list:
	vfree(umem_odp->page_list);
	return ret_val;
}