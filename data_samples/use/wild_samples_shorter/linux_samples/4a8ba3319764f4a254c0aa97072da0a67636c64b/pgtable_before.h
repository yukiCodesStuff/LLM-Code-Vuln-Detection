#define pfn_pmd(pfn,prot)	(__pmd(((phys_addr_t)(pfn) << PAGE_SHIFT) | pgprot_val(prot)))
#define mk_pmd(page,prot)	pfn_pmd(page_to_pfn(page),prot)

#define pmd_page(pmd)           pfn_to_page(__phys_to_pfn(pmd_val(pmd) & PHYS_MASK))
#define pud_write(pud)		pte_write(pud_pte(pud))
#define pud_pfn(pud)		(((pud_val(pud) & PUD_MASK) & PHYS_MASK) >> PAGE_SHIFT)

#define set_pmd_at(mm, addr, pmdp, pmd)	set_pte_at(mm, addr, (pte_t *)pmdp, pmd_pte(pmd))
	return (pmd_t *)pud_page_vaddr(*pud) + pmd_index(addr);
}

#define pud_page(pud)           pmd_page(pud_pmd(pud))

#endif	/* CONFIG_ARM64_PGTABLE_LEVELS > 2 */

#if CONFIG_ARM64_PGTABLE_LEVELS > 3
	return (pud_t *)pgd_page_vaddr(*pgd) + pud_index(addr);
}

#endif  /* CONFIG_ARM64_PGTABLE_LEVELS > 3 */

#define pgd_ERROR(pgd)		__pgd_error(__FILE__, __LINE__, pgd_val(pgd))
