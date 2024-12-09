{
	struct mm_struct *mm = arg;

	if (current->active_mm == mm)
		set_user_asce(mm);
	__tlb_flush_local();
}

int crst_table_upgrade(struct mm_struct *mm, unsigned long end)
{
	unsigned long *pgd = NULL, *p4d = NULL, *__pgd;
	unsigned long asce_limit = mm->context.asce_limit;

	/* upgrade should only happen from 3 to 4, 3 to 5, or 4 to 5 levels */
	VM_BUG_ON(asce_limit < _REGION2_SIZE);

	if (end <= asce_limit)
		return 0;

	if (asce_limit == _REGION2_SIZE) {
		p4d = crst_table_alloc(mm);
		if (unlikely(!p4d))
			goto err_p4d;
		crst_table_init(p4d, _REGION2_ENTRY_EMPTY);
	}