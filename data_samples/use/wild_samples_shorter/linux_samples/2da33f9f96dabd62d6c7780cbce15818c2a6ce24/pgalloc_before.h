
static inline pgd_t *pgd_alloc(struct mm_struct *mm)
{
	spin_lock_init(&mm->context.list_lock);
	INIT_LIST_HEAD(&mm->context.pgtable_list);
	INIT_LIST_HEAD(&mm->context.gmap_list);
	return (pgd_t *) crst_table_alloc(mm);
}
#define pgd_free(mm, pgd) crst_table_free(mm, (unsigned long *) pgd)

static inline void pmd_populate(struct mm_struct *mm,
				pmd_t *pmd, pgtable_t pte)
{