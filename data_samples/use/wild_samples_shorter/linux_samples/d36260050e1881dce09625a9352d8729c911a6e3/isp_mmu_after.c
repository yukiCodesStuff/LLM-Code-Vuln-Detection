		/*
		 * setup L1 page table physical addr to MMU
		 */
		mmu->base_address = l1_pt;
		mmu->l1_pte = isp_pgaddr_to_pte_valid(mmu, l1_pt);
		memset(mmu->l2_pgt_refcount, 0, sizeof(int) * ISP_L1PT_PTES);
	}

	mmu->driver = driver;

	if (!driver->tlb_flush_all) {
		dev_err(atomisp_dev, "tlb_flush_all operation not provided.\n");
		return -EINVAL;
	}

	if (!driver->tlb_flush_range)