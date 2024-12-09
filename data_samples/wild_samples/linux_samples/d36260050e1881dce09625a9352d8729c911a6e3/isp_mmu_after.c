		if (l1_pt == NULL_PAGE) {
			dev_err(atomisp_dev, "alloc page table fail.\n");
			mutex_unlock(&mmu->pt_mutex);
			return -ENOMEM;
		}

		/*
		 * setup L1 page table physical addr to MMU
		 */
		mmu->base_address = l1_pt;
		mmu->l1_pte = isp_pgaddr_to_pte_valid(mmu, l1_pt);
		memset(mmu->l2_pgt_refcount, 0, sizeof(int) * ISP_L1PT_PTES);
	}

	l1_pt = isp_pte_to_pgaddr(mmu, mmu->l1_pte);

	start = (isp_virt) & ISP_PAGE_MASK;
	end = start + (pgnr << ISP_PAGE_OFFSET);
	phys &= ISP_PAGE_MASK;

	ret = mmu_l1_map(mmu, l1_pt, start, end, phys);

	if (ret)
		dev_err(atomisp_dev, "setup mapping in L1PT fail.\n");

	mutex_unlock(&mmu->pt_mutex);
	return ret;
}

/*
 * Free L2 page table according to isp virtual address and page physical
 * address
 */
static void mmu_l2_unmap(struct isp_mmu *mmu, phys_addr_t l1_pt,
			   unsigned int l1_idx, phys_addr_t l2_pt,
			   unsigned int start, unsigned int end)
{

	unsigned int ptr;
	unsigned int idx;
	unsigned int pte;

	l2_pt &= ISP_PAGE_MASK;

	start = start & ISP_PAGE_MASK;
	end = ISP_PAGE_ALIGN(end);

	ptr = start;
	do {
		idx = ISP_PTR_TO_L2_IDX(ptr);

		pte = atomisp_get_pte(l2_pt, idx);

		if (!ISP_PTE_VALID(mmu, pte))
			mmu_unmap_l2_pte_error(mmu, l1_pt, l1_idx,
						 l2_pt, idx, ptr, pte);

		atomisp_set_pte(l2_pt, idx, mmu->driver->null_pte);
		mmu->l2_pgt_refcount[l1_idx]--;
		ptr += (1U << ISP_L2PT_OFFSET);
	} while (ptr < end && idx < ISP_L2PT_PTES - 1);

	if (mmu->l2_pgt_refcount[l1_idx] == 0) {
		free_page_table(mmu, l2_pt);
		atomisp_set_pte(l1_pt, l1_idx, mmu->driver->null_pte);
	}
{
	if (!mmu)		/* error */
		return -EINVAL;
	if (!driver)		/* error */
		return -EINVAL;

	if (!driver->name)
		dev_warn(atomisp_dev, "NULL name for MMU driver...\n");

	mmu->driver = driver;

	if (!driver->tlb_flush_all) {
		dev_err(atomisp_dev, "tlb_flush_all operation not provided.\n");
		return -EINVAL;
	}