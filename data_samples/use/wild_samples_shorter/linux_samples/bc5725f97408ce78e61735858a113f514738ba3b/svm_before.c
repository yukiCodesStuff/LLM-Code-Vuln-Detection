	return ret;
}

static int get_num_contig_pages(int idx, struct page **inpages,
				unsigned long npages)
{
	unsigned long paddr, next_paddr;
	int i = idx + 1, pages = 1;

	/* find the number of contiguous pages starting from idx */
	paddr = __sme_page_pa(inpages[idx]);
	while (i < npages) {

static int sev_launch_update_data(struct kvm *kvm, struct kvm_sev_cmd *argp)
{
	unsigned long vaddr, vaddr_end, next_vaddr, npages, size;
	struct kvm_sev_info *sev = &to_kvm_svm(kvm)->sev_info;
	struct kvm_sev_launch_update_data params;
	struct sev_data_launch_update_data *data;
	struct page **inpages;
	int i, ret, pages;

	if (!sev_guest(kvm))
		return -ENOTTY;

	struct page **src_p, **dst_p;
	struct kvm_sev_dbg debug;
	unsigned long n;
	int ret, size;

	if (!sev_guest(kvm))
		return -ENOTTY;

	if (copy_from_user(&debug, (void __user *)(uintptr_t)argp->data, sizeof(debug)))
		return -EFAULT;

	vaddr = debug.src_uaddr;
	size = debug.len;
	vaddr_end = vaddr + size;
	dst_vaddr = debug.dst_uaddr;
						     dst_vaddr,
						     len, &argp->error);

		sev_unpin_memory(kvm, src_p, 1);
		sev_unpin_memory(kvm, dst_p, 1);

		if (ret)
			goto err;
