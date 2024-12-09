{
	native_set_pte(ptep, pte);
}

static inline void set_pmd_at(struct mm_struct *mm, unsigned long addr,
			      pmd_t *pmdp, pmd_t pmd)
{
	set_pmd(pmdp, pmd);
}