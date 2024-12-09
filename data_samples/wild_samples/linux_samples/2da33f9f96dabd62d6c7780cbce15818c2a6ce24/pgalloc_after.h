{
	pud_val(*pud) = _REGION3_ENTRY | __pa(pmd);
}

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