		unsigned long vpn  = hpt_vpn(vaddr, vsid, ssize);
		unsigned long tprot = prot;

		/* Make kernel text executable */
		if (overlaps_kernel_text(vaddr, vaddr + step))
			tprot &= ~HPTE_R_N;

	/* Initialize stab / SLB management */
	if (mmu_has_feature(MMU_FTR_SLB))
		slb_initialize();
}

#ifdef CONFIG_SMP
void __cpuinit early_init_mmu_secondary(void)
	DBG_LOW("hash_page(ea=%016lx, access=%lx, trap=%lx\n",
		ea, access, trap);

	if ((ea & ~REGION_MASK) >= PGTABLE_RANGE) {
		DBG_LOW(" out of pgtable range !\n");
 		return 1;
	}

	/* Get region & vsid */
 	switch (REGION_ID(ea)) {
	case USER_REGION_ID:
		user_region = 1;
	}
	DBG_LOW(" mm=%p, mm->pgdir=%p, vsid=%016lx\n", mm, mm->pgd, vsid);

	/* Get pgdir */
	pgdir = mm->pgd;
	if (pgdir == NULL)
		return 1;
	/* Get VSID */
	ssize = user_segment_size(ea);
	vsid = get_vsid(mm->context.id, ea, ssize);

	/* Hash doesn't like irqs */
	local_irq_save(flags);

	hash = hpt_hash(vpn, PAGE_SHIFT, mmu_kernel_ssize);
	hpteg = ((hash & htab_hash_mask) * HPTES_PER_GROUP);

	ret = ppc_md.hpte_insert(hpteg, vpn, __pa(vaddr),
				 mode, HPTE_V_BOLTED,
				 mmu_linear_psize, mmu_kernel_ssize);
	BUG_ON (ret < 0);