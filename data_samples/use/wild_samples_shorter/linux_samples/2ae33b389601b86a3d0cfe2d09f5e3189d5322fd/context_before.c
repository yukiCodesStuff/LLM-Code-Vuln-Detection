	return 0;
}

static void new_context(struct mm_struct *mm, unsigned int cpu)
{
	u64 asid = mm->context.id;
	u64 generation = atomic64_read(&asid_generation);

	if (asid != 0 && is_reserved_asid(asid)) {
		/*
		cpumask_clear(mm_cpumask(mm));
	}

	mm->context.id = asid;
}

void check_and_switch_context(struct mm_struct *mm, struct task_struct *tsk)
{
	unsigned long flags;
	unsigned int cpu = smp_processor_id();

	if (unlikely(mm->context.vmalloc_seq != init_mm.context.vmalloc_seq))
		__check_vmalloc_seq(mm);

	 */
	cpu_set_reserved_ttbr0();

	if (!((mm->context.id ^ atomic64_read(&asid_generation)) >> ASID_BITS)
	    && atomic64_xchg(&per_cpu(active_asids, cpu), mm->context.id))
		goto switch_mm_fastpath;

	raw_spin_lock_irqsave(&cpu_asid_lock, flags);
	/* Check that our ASID belongs to the current generation. */
	if ((mm->context.id ^ atomic64_read(&asid_generation)) >> ASID_BITS)
		new_context(mm, cpu);

	atomic64_set(&per_cpu(active_asids, cpu), mm->context.id);
	cpumask_set_cpu(cpu, mm_cpumask(mm));

	if (cpumask_test_and_clear_cpu(cpu, &tlb_flush_pending))
		local_flush_tlb_all();
	raw_spin_unlock_irqrestore(&cpu_asid_lock, flags);

switch_mm_fastpath:
	cpu_switch_mm(mm->pgd, mm);