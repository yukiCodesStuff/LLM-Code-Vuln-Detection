	unsigned int null_pte;

	/*
	 * set/get page directory base address (physical address).
	 *
	 * must be provided.
	 */
	int (*set_pd_base) (struct isp_mmu *mmu,
			phys_addr_t pd_base);
	unsigned int (*get_pd_base) (struct isp_mmu *mmu, phys_addr_t pd_base);
	/*
	 * callback to flush tlb.
	 *