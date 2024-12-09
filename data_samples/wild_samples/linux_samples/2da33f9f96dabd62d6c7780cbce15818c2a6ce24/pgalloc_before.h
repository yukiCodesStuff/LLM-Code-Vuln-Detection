{
	pud_val(*pud) = _REGION3_ENTRY | __pa(pmd);
}

static inline pgd_t *pgd_alloc(struct mm_struct *mm)
{
	spin_lock_init(&mm->context.list_lock);
	INIT_LIST_HEAD(&mm->context.pgtable_list);
	INIT_LIST_HEAD(&mm->context.gmap_list);
	return (pgd_t *) crst_table_alloc(mm);
}