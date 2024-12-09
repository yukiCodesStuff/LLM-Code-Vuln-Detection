{
	/*
	 * Hash maps the memory with one size mmu_linear_psize.
	 * So don't bother to print these on hash
	 */
	if (!radix_enabled())
		return;
	seq_printf(m, "DirectMap4k:    %8lu kB\n",
		   atomic_long_read(&direct_pages_count[MMU_PAGE_4K]) << 2);
	seq_printf(m, "DirectMap64k:    %8lu kB\n",
		   atomic_long_read(&direct_pages_count[MMU_PAGE_64K]) << 6);
	seq_printf(m, "DirectMap2M:    %8lu kB\n",
		   atomic_long_read(&direct_pages_count[MMU_PAGE_2M]) << 11);
	seq_printf(m, "DirectMap1G:    %8lu kB\n",
		   atomic_long_read(&direct_pages_count[MMU_PAGE_1G]) << 20);
}
#endif /* CONFIG_PROC_FS */

/*
 * For hash translation mode, we use the deposited table to store hash slot
 * information and they are stored at PTRS_PER_PMD offset from related pmd
 * location. Hence a pmd move requires deposit and withdraw.
 *
 * For radix translation with split pmd ptl, we store the deposited table in the
 * pmd page. Hence if we have different pmd page we need to withdraw during pmd
 * move.
 *
 * With hash we use deposited table always irrespective of anon or not.
 * With radix we use deposited table only for anonymous mapping.
 */
int pmd_move_must_withdraw(struct spinlock *new_pmd_ptl,
			   struct spinlock *old_pmd_ptl,
			   struct vm_area_struct *vma)
{
	if (radix_enabled())
		return (new_pmd_ptl != old_pmd_ptl) && vma_is_anonymous(vma);

	return true;
}