static inline int init_new_context(struct task_struct *tsk,
				   struct mm_struct *mm)
{
	cpumask_clear(&mm->context.cpu_attach_mask);
	atomic_set(&mm->context.attach_count, 0);
	mm->context.flush_mm = 0;
	mm->context.asce_bits = _ASCE_TABLE_LENGTH | _ASCE_USER_BITS;
	mm->context.asce_bits |= _ASCE_TYPE_REGION3;
#ifdef CONFIG_PGSTE
	mm->context.alloc_pgste = page_table_allocate_pgste;
	mm->context.has_pgste = 0;
	mm->context.use_skey = 0;
#endif
	mm->context.asce_limit = STACK_TOP_MAX;
	crst_table_init((unsigned long *) mm->pgd, pgd_entry_type(mm));
	return 0;
}

static inline void arch_dup_mmap(struct mm_struct *oldmm,
				 struct mm_struct *mm)
{
	if (oldmm->context.asce_limit < mm->context.asce_limit)
		crst_table_downgrade(mm, oldmm->context.asce_limit);
}

static inline void arch_exit_mmap(struct mm_struct *mm)
{