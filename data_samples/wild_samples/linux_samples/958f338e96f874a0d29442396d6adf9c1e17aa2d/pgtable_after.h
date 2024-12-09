{
	return 0;
}
#endif	/* CONFIG_HAVE_ARCH_HUGE_VMAP */

#ifndef __HAVE_ARCH_FLUSH_PMD_TLB_RANGE
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
/*
 * ARCHes with special requirements for evicting THP backing TLB entries can
 * implement this. Otherwise also, it can help optimize normal TLB flush in
 * THP regime. stock flush_tlb_range() typically has optimization to nuke the
 * entire TLB TLB if flush span is greater than a threshold, which will
 * likely be true for a single huge page. Thus a single thp flush will
 * invalidate the entire TLB which is not desitable.
 * e.g. see arch/arc: flush_pmd_tlb_range
 */
#define flush_pmd_tlb_range(vma, addr, end)	flush_tlb_range(vma, addr, end)
#define flush_pud_tlb_range(vma, addr, end)	flush_tlb_range(vma, addr, end)
#else
#define flush_pmd_tlb_range(vma, addr, end)	BUILD_BUG()
#define flush_pud_tlb_range(vma, addr, end)	BUILD_BUG()
#endif
#endif

struct file;
int phys_mem_access_prot_allowed(struct file *file, unsigned long pfn,
			unsigned long size, pgprot_t *vma_prot);

#ifndef CONFIG_X86_ESPFIX64
static inline void init_espfix_bsp(void) { }
#endif

#ifndef __HAVE_ARCH_PFN_MODIFY_ALLOWED
static inline bool pfn_modify_allowed(unsigned long pfn, pgprot_t prot)
{
	return true;
}