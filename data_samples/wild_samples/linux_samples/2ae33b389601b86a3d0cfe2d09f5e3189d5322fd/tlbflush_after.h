		: : "a" (2048), "a" (asce) : "cc" );
}

static inline void __tlb_flush_mm(struct mm_struct * mm)
{
	/*
	 * If the machine has IDTE we prefer to do a per mm flush
	 * on all cpus instead of doing a local flush if the mm
	 * only ran on the local cpu.
	 */
	if (MACHINE_HAS_IDTE && list_empty(&mm->context.gmap_list))
		__tlb_flush_idte((unsigned long) mm->pgd |
				 mm->context.asce_bits);
	else
		__tlb_flush_full(mm);
}