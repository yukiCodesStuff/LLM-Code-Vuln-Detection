{
	struct mm_struct *mm = arg;

	if (current->active_mm == mm)
		set_user_asce(mm);
	__tlb_flush_local();
}

int crst_table_upgrade(struct mm_struct *mm, unsigned long end)
{
	unsigned long *table, *pgd;
	int rc, notify;

	/* upgrade should only happen from 3 to 4, 3 to 5, or 4 to 5 levels */
	VM_BUG_ON(mm->context.asce_limit < _REGION2_SIZE);
	rc = 0;
	notify = 0;
	while (mm->context.asce_limit < end) {
		table = crst_table_alloc(mm);
		if (!table) {
			rc = -ENOMEM;
			break;
		}
		spin_lock_bh(&mm->page_table_lock);
		pgd = (unsigned long *) mm->pgd;
		if (mm->context.asce_limit == _REGION2_SIZE) {
			crst_table_init(table, _REGION2_ENTRY_EMPTY);
			p4d_populate(mm, (p4d_t *) table, (pud_t *) pgd);
			mm->pgd = (pgd_t *) table;
			mm->context.asce_limit = _REGION1_SIZE;
			mm->context.asce = __pa(mm->pgd) | _ASCE_TABLE_LENGTH |
				_ASCE_USER_BITS | _ASCE_TYPE_REGION2;
			mm_inc_nr_puds(mm);
		} else {
			crst_table_init(table, _REGION1_ENTRY_EMPTY);
			pgd_populate(mm, (pgd_t *) table, (p4d_t *) pgd);
			mm->pgd = (pgd_t *) table;
			mm->context.asce_limit = -PAGE_SIZE;
			mm->context.asce = __pa(mm->pgd) | _ASCE_TABLE_LENGTH |
				_ASCE_USER_BITS | _ASCE_TYPE_REGION1;
		}
		notify = 1;
		spin_unlock_bh(&mm->page_table_lock);
	}
	if (notify)
		on_each_cpu(__crst_table_upgrade, mm, 0);
	return rc;
}