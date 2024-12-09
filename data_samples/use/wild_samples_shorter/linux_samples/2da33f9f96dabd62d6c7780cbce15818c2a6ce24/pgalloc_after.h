
static inline pgd_t *pgd_alloc(struct mm_struct *mm)
{
	unsigned long *table = crst_table_alloc(mm);

	if (!table)
		return NULL;
	if (mm->context.asce_limit == (1UL << 31)) {
		/* Forking a compat process with 2 page table levels */
		if (!pgtable_pmd_page_ctor(virt_to_page(table))) {
			crst_table_free(mm, table);
			return NULL;
		}
	}
	return (pgd_t *) table;
}

static inline void pgd_free(struct mm_struct *mm, pgd_t *pgd)
{
	if (mm->context.asce_limit == (1UL << 31))
		pgtable_pmd_page_dtor(virt_to_page(pgd));
	crst_table_free(mm, (unsigned long *) pgd);
}

static inline void pmd_populate(struct mm_struct *mm,
				pmd_t *pmd, pgtable_t pte)
{