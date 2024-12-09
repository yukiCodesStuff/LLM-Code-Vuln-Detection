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