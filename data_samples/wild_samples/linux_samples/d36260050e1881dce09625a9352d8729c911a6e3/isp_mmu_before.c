		if (l1_pt == NULL_PAGE) {
			dev_err(atomisp_dev, "alloc page table fail.\n");
			mutex_unlock(&mmu->pt_mutex);
			return -ENOMEM;
		}

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
{
	if (!mmu)		/* error */
		return -EINVAL;
	if (!driver)		/* error */
		return -EINVAL;

	if (!driver->name)
		dev_warn(atomisp_dev, "NULL name for MMU driver...\n");

	mmu->driver = driver;

	if (!driver->set_pd_base || !driver->tlb_flush_all) {
		dev_err(atomisp_dev,
			    "set_pd_base or tlb_flush_all operation "
			     "not provided.\n");
		return -EINVAL;
	}