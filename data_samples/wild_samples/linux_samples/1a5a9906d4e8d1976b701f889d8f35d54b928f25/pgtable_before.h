{
}
#else
extern int track_pfn_vma_new(struct vm_area_struct *vma, pgprot_t *prot,
				unsigned long pfn, unsigned long size);
extern int track_pfn_vma_copy(struct vm_area_struct *vma);
extern void untrack_pfn_vma(struct vm_area_struct *vma, unsigned long pfn,
				unsigned long size);
#endif

#ifndef CONFIG_TRANSPARENT_HUGEPAGE
static inline int pmd_trans_huge(pmd_t pmd)
{
	return 0;
}
{
	BUG();
	return 0;
}
#endif /* __HAVE_ARCH_PMD_WRITE */
#endif

#endif /* !__ASSEMBLY__ */

#endif /* _ASM_GENERIC_PGTABLE_H */