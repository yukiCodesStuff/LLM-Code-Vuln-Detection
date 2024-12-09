	     vaddr += step, paddr += step) {
		unsigned long hash, hpteg;
		unsigned long vsid = get_kernel_vsid(vaddr, ssize);
		unsigned long vpn  = hpt_vpn(vaddr, vsid, ssize);
		unsigned long tprot = prot;

		/* Make kernel text executable */
		if (overlaps_kernel_text(vaddr, vaddr + step))
			tprot &= ~HPTE_R_N;

		hash = hpt_hash(vpn, shift, ssize);
		hpteg = ((hash & htab_hash_mask) * HPTES_PER_GROUP);

		BUG_ON(!ppc_md.hpte_insert);
		ret = ppc_md.hpte_insert(hpteg, vpn, paddr, tprot,
					 HPTE_V_BOLTED, psize, ssize);

		if (ret < 0)
			break;
#ifdef CONFIG_DEBUG_PAGEALLOC
		if ((paddr >> PAGE_SHIFT) < linear_map_hash_count)
			linear_map_hash_slots[paddr >> PAGE_SHIFT] = ret | 0x80;
#endif /* CONFIG_DEBUG_PAGEALLOC */
	}
	return ret < 0 ? ret : 0;
}

#ifdef CONFIG_MEMORY_HOTPLUG
static int htab_remove_mapping(unsigned long vstart, unsigned long vend,
		      int psize, int ssize)
{
	unsigned long vaddr;
	unsigned int step, shift;

	shift = mmu_psize_defs[psize].shift;
	step = 1 << shift;

	if (!ppc_md.hpte_removebolted) {
		printk(KERN_WARNING "Platform doesn't implement "
				"hpte_removebolted\n");
		return -EINVAL;
	}

	for (vaddr = vstart; vaddr < vend; vaddr += step)
		ppc_md.hpte_removebolted(vaddr, psize, ssize);

	return 0;
}
#endif /* CONFIG_MEMORY_HOTPLUG */

static int __init htab_dt_scan_seg_sizes(unsigned long node,
					 const char *uname, int depth,
					 void *data)
{
	char *type = of_get_flat_dt_prop(node, "device_type", NULL);
	u32 *prop;
	unsigned long size = 0;

	/* We are scanning "cpu" nodes only */
	if (type == NULL || strcmp(type, "cpu") != 0)
		return 0;

	prop = (u32 *)of_get_flat_dt_prop(node, "ibm,processor-segment-sizes",
					  &size);
	if (prop == NULL)
		return 0;
	for (; size >= 4; size -= 4, ++prop) {
		if (prop[0] == 40) {
			DBG("1T segment support detected\n");
			cur_cpu_spec->mmu_features |= MMU_FTR_1T_SEGMENT;
			return 1;
		}
{
	/* Setup initial STAB address in the PACA */
	get_paca()->stab_real = __pa((u64)&initial_stab);
	get_paca()->stab_addr = (u64)&initial_stab;

	/* Initialize the MMU Hash table and create the linear mapping
	 * of memory. Has to be done before stab/slb initialization as
	 * this is currently where the page size encoding is obtained
	 */
	htab_initialize();

	/* Initialize stab / SLB management */
	if (mmu_has_feature(MMU_FTR_SLB))
		slb_initialize();
}

#ifdef CONFIG_SMP
void __cpuinit early_init_mmu_secondary(void)
{
	/* Initialize hash table for that CPU */
	if (!firmware_has_feature(FW_FEATURE_LPAR))
		mtspr(SPRN_SDR1, _SDR1);

	/* Initialize STAB/SLB. We use a virtual address as it works
	 * in real mode on pSeries.
	 */
	if (mmu_has_feature(MMU_FTR_SLB))
		slb_initialize();
	else
		stab_initialize(get_paca()->stab_addr);
}
#endif /* CONFIG_SMP */

/*
 * Called by asm hashtable.S for doing lazy icache flush
 */
unsigned int hash_page_do_lazy_icache(unsigned int pp, pte_t pte, int trap)
{
	struct page *page;

	if (!pfn_valid(pte_pfn(pte)))
		return pp;

	page = pte_page(pte);

	/* page is dirty */
	if (!test_bit(PG_arch_1, &page->flags) && !PageReserved(page)) {
		if (trap == 0x400) {
			flush_dcache_icache_page(page);
			set_bit(PG_arch_1, &page->flags);
		} else
			pp |= HPTE_R_N;
	}
	return pp;
}
{
	pgd_t *pgdir;
	unsigned long vsid;
	struct mm_struct *mm;
	pte_t *ptep;
	unsigned hugeshift;
	const struct cpumask *tmp;
	int rc, user_region = 0, local = 0;
	int psize, ssize;

	DBG_LOW("hash_page(ea=%016lx, access=%lx, trap=%lx\n",
		ea, access, trap);

	if ((ea & ~REGION_MASK) >= PGTABLE_RANGE) {
		DBG_LOW(" out of pgtable range !\n");
 		return 1;
	}
		if (! mm) {
			DBG_LOW(" user region with no mm !\n");
			return 1;
		}
		psize = get_slice_psize(mm, ea);
		ssize = user_segment_size(ea);
		vsid = get_vsid(mm->context.id, ea, ssize);
		break;
	case VMALLOC_REGION_ID:
		mm = &init_mm;
		vsid = get_kernel_vsid(ea, mmu_kernel_ssize);
		if (ea < VMALLOC_END)
			psize = mmu_vmalloc_psize;
		else
			psize = mmu_io_psize;
		ssize = mmu_kernel_ssize;
		break;
	default:
		/* Not a valid range
		 * Send the problem up to do_page_fault 
		 */
		return 1;
	}
	DBG_LOW(" mm=%p, mm->pgdir=%p, vsid=%016lx\n", mm, mm->pgd, vsid);

	/* Get pgdir */
	pgdir = mm->pgd;
	if (pgdir == NULL)
		return 1;

	/* Check CPU locality */
	tmp = cpumask_of(smp_processor_id());
	if (user_region && cpumask_equal(mm_cpumask(mm), tmp))
		local = 1;

#ifndef CONFIG_PPC_64K_PAGES
	/* If we use 4K pages and our psize is not 4K, then we might
	 * be hitting a special driver mapping, and need to align the
	 * address before we fetch the PTE.
	 *
	 * It could also be a hugepage mapping, in which case this is
	 * not necessary, but it's not harmful, either.
	 */
	if (psize != MMU_PAGE_4K)
		ea &= ~((1ul << mmu_psize_defs[psize].shift) - 1);
#endif /* CONFIG_PPC_64K_PAGES */

	/* Get PTE and page size from page tables */
	ptep = find_linux_pte_or_hugepte(pgdir, ea, &hugeshift);
	if (ptep == NULL || !pte_present(*ptep)) {
		DBG_LOW(" no PTE !\n");
		return 1;
	}
{
	unsigned long vsid;
	pgd_t *pgdir;
	pte_t *ptep;
	unsigned long flags;
	int rc, ssize, local = 0;

	BUG_ON(REGION_ID(ea) != USER_REGION_ID);

#ifdef CONFIG_PPC_MM_SLICES
	/* We only prefault standard pages for now */
	if (unlikely(get_slice_psize(mm, ea) != mm->context.user_psize))
		return;
#endif

	DBG_LOW("hash_preload(mm=%p, mm->pgdir=%p, ea=%016lx, access=%lx,"
		" trap=%lx\n", mm, mm->pgd, ea, access, trap);

	/* Get Linux PTE if available */
	pgdir = mm->pgd;
	if (pgdir == NULL)
		return;
	ptep = find_linux_pte(pgdir, ea);
	if (!ptep)
		return;

#ifdef CONFIG_PPC_64K_PAGES
	/* If either _PAGE_4K_PFN or _PAGE_NO_CACHE is set (and we are on
	 * a 64K kernel), then we don't preload, hash_page() will take
	 * care of it once we actually try to access the page.
	 * That way we don't have to duplicate all of the logic for segment
	 * page size demotion here
	 */
	if (pte_val(*ptep) & (_PAGE_4K_PFN | _PAGE_NO_CACHE))
		return;
#endif /* CONFIG_PPC_64K_PAGES */

	/* Get VSID */
	ssize = user_segment_size(ea);
	vsid = get_vsid(mm->context.id, ea, ssize);

	/* Hash doesn't like irqs */
	local_irq_save(flags);

	/* Is that local to this CPU ? */
	if (cpumask_equal(mm_cpumask(mm), cpumask_of(smp_processor_id())))
		local = 1;

	/* Hash it in */
#ifdef CONFIG_PPC_HAS_HASH_64K
	if (mm->context.user_psize == MMU_PAGE_64K)
		rc = __hash_page_64K(ea, access, vsid, ptep, trap, local, ssize);
	else
#endif /* CONFIG_PPC_HAS_HASH_64K */
		rc = __hash_page_4K(ea, access, vsid, ptep, trap, local, ssize,
				    subpage_protection(mm, ea));

	/* Dump some info in case of hash insertion failure, they should
	 * never happen so it is really useful to know if/when they do
	 */
	if (rc == -1)
		hash_failure_debug(ea, access, vsid, trap, ssize,
				   mm->context.user_psize, pte_val(*ptep));

	local_irq_restore(flags);
}

/* WARNING: This is called from hash_low_64.S, if you change this prototype,
 *          do not forget to update the assembly call site !
 */
void flush_hash_page(unsigned long vpn, real_pte_t pte, int psize, int ssize,
		     int local)
{
	unsigned long hash, index, shift, hidx, slot;

	DBG_LOW("flush_hash_page(vpn=%016lx)\n", vpn);
	pte_iterate_hashed_subpages(pte, psize, vpn, index, shift) {
		hash = hpt_hash(vpn, shift, ssize);
		hidx = __rpte_to_hidx(pte, index);
		if (hidx & _PTEIDX_SECONDARY)
			hash = ~hash;
		slot = (hash & htab_hash_mask) * HPTES_PER_GROUP;
		slot += hidx & _PTEIDX_GROUP_IX;
		DBG_LOW(" sub %ld: hash=%lx, hidx=%lx\n", index, slot, hidx);
		ppc_md.hpte_invalidate(slot, vpn, psize, ssize, local);
	} pte_iterate_hashed_end();

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	/* Transactions are not aborted by tlbiel, only tlbie.
	 * Without, syncing a page back to a block device w/ PIO could pick up
	 * transactional data (bad!) so we force an abort here.  Before the
	 * sync the page will be made read-only, which will flush_hash_page.
	 * BIG ISSUE here: if the kernel uses a page from userspace without
	 * unmapping it first, it may see the speculated version.
	 */
	if (local && cpu_has_feature(CPU_FTR_TM) &&
	    MSR_TM_ACTIVE(current->thread.regs->msr)) {
		tm_enable();
		tm_abort(TM_CAUSE_TLBI);
	}
#endif
}
{
	unsigned long hash, hpteg;
	unsigned long vsid = get_kernel_vsid(vaddr, mmu_kernel_ssize);
	unsigned long vpn = hpt_vpn(vaddr, vsid, mmu_kernel_ssize);
	unsigned long mode = htab_convert_pte_flags(PAGE_KERNEL);
	int ret;

	hash = hpt_hash(vpn, PAGE_SHIFT, mmu_kernel_ssize);
	hpteg = ((hash & htab_hash_mask) * HPTES_PER_GROUP);

	ret = ppc_md.hpte_insert(hpteg, vpn, __pa(vaddr),
				 mode, HPTE_V_BOLTED,
				 mmu_linear_psize, mmu_kernel_ssize);
	BUG_ON (ret < 0);
	spin_lock(&linear_map_hash_lock);
	BUG_ON(linear_map_hash_slots[lmi] & 0x80);
	linear_map_hash_slots[lmi] = ret | 0x80;
	spin_unlock(&linear_map_hash_lock);
}

static void kernel_unmap_linear_page(unsigned long vaddr, unsigned long lmi)
{
	unsigned long hash, hidx, slot;
	unsigned long vsid = get_kernel_vsid(vaddr, mmu_kernel_ssize);
	unsigned long vpn = hpt_vpn(vaddr, vsid, mmu_kernel_ssize);

	hash = hpt_hash(vpn, PAGE_SHIFT, mmu_kernel_ssize);
	spin_lock(&linear_map_hash_lock);
	BUG_ON(!(linear_map_hash_slots[lmi] & 0x80));
	hidx = linear_map_hash_slots[lmi] & 0x7f;
	linear_map_hash_slots[lmi] = 0;
	spin_unlock(&linear_map_hash_lock);
	if (hidx & _PTEIDX_SECONDARY)
		hash = ~hash;
	slot = (hash & htab_hash_mask) * HPTES_PER_GROUP;
	slot += hidx & _PTEIDX_GROUP_IX;
	ppc_md.hpte_invalidate(slot, vpn, mmu_linear_psize, mmu_kernel_ssize, 0);
}

void kernel_map_pages(struct page *page, int numpages, int enable)
{
	unsigned long flags, vaddr, lmi;
	int i;

	local_irq_save(flags);
	for (i = 0; i < numpages; i++, page++) {
		vaddr = (unsigned long)page_address(page);
		lmi = __pa(vaddr) >> PAGE_SHIFT;
		if (lmi >= linear_map_hash_count)
			continue;
		if (enable)
			kernel_map_linear_page(vaddr, lmi);
		else
			kernel_unmap_linear_page(vaddr, lmi);
	}