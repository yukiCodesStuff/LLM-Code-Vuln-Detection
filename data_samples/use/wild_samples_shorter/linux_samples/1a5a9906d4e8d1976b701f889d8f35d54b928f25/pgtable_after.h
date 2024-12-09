				unsigned long size);
#endif

#ifdef CONFIG_MMU

#ifndef CONFIG_TRANSPARENT_HUGEPAGE
static inline int pmd_trans_huge(pmd_t pmd)
{
	return 0;
	return 0;
}
#endif /* __HAVE_ARCH_PMD_WRITE */
#endif /* CONFIG_TRANSPARENT_HUGEPAGE */

/*
 * This function is meant to be used by sites walking pagetables with
 * the mmap_sem hold in read mode to protect against MADV_DONTNEED and
 * transhuge page faults. MADV_DONTNEED can convert a transhuge pmd
 * into a null pmd and the transhuge page fault can convert a null pmd
 * into an hugepmd or into a regular pmd (if the hugepage allocation
 * fails). While holding the mmap_sem in read mode the pmd becomes
 * stable and stops changing under us only if it's not null and not a
 * transhuge pmd. When those races occurs and this function makes a
 * difference vs the standard pmd_none_or_clear_bad, the result is
 * undefined so behaving like if the pmd was none is safe (because it
 * can return none anyway). The compiler level barrier() is critically
 * important to compute the two checks atomically on the same pmdval.
 */
static inline int pmd_none_or_trans_huge_or_clear_bad(pmd_t *pmd)
{
	/* depend on compiler for an atomic pmd read */
	pmd_t pmdval = *pmd;
	/*
	 * The barrier will stabilize the pmdval in a register or on
	 * the stack so that it will stop changing under the code.
	 */
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	barrier();
#endif
	if (pmd_none(pmdval))
		return 1;
	if (unlikely(pmd_bad(pmdval))) {
		if (!pmd_trans_huge(pmdval))
			pmd_clear_bad(pmd);
		return 1;
	}
	return 0;
}

/*
 * This is a noop if Transparent Hugepage Support is not built into
 * the kernel. Otherwise it is equivalent to
 * pmd_none_or_trans_huge_or_clear_bad(), and shall only be called in
 * places that already verified the pmd is not none and they want to
 * walk ptes while holding the mmap sem in read mode (write mode don't
 * need this). If THP is not enabled, the pmd can't go away under the
 * code even if MADV_DONTNEED runs, but if THP is enabled we need to
 * run a pmd_trans_unstable before walking the ptes after
 * split_huge_page_pmd returns (because it may have run when the pmd
 * become null, but then a page fault can map in a THP and not a
 * regular page).
 */
static inline int pmd_trans_unstable(pmd_t *pmd)
{
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	return pmd_none_or_trans_huge_or_clear_bad(pmd);
#else
	return 0;
#endif
}

#endif /* CONFIG_MMU */

#endif /* !__ASSEMBLY__ */

#endif /* _ASM_GENERIC_PGTABLE_H */