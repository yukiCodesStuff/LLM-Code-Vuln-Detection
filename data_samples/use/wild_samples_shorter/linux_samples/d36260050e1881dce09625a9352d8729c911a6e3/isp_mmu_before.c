		/*
		 * setup L1 page table physical addr to MMU
		 */
		ret = mmu->driver->set_pd_base(mmu, l1_pt);
		if (ret) {
			dev_err(atomisp_dev,
				 "set page directory base address fail.\n");
			mutex_unlock(&mmu->pt_mutex);
			return ret;
		}
		mmu->base_address = l1_pt;
		mmu->l1_pte = isp_pgaddr_to_pte_valid(mmu, l1_pt);
		memset(mmu->l2_pgt_refcount, 0, sizeof(int) * ISP_L1PT_PTES);
	}

	mmu->driver = driver;

	if (!driver->set_pd_base || !driver->tlb_flush_all) {
		dev_err(atomisp_dev,
			    "set_pd_base or tlb_flush_all operation "
			     "not provided.\n");
		return -EINVAL;
	}

	if (!driver->tlb_flush_range)