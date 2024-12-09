		for (v = 0; v < N_EXCEPTION_STACKS; v++) {
			estacks += exception_stack_sizes[v];
			oist->ist[v] = t->x86_tss.ist[v] =
					(unsigned long)estacks;
			if (v == DEBUG_STACK-1)
				per_cpu(debug_stack_addr, cpu) = (unsigned long)estacks;
		}
	}

	t->x86_tss.io_bitmap_base = offsetof(struct tss_struct, io_bitmap);

	/*
	 * <= is required because the CPU will access up to
	 * 8 bits beyond the end of the IO permission bitmap.
	 */
	for (i = 0; i <= IO_BITMAP_LONGS; i++)
		t->io_bitmap[i] = ~0UL;

	atomic_inc(&init_mm.mm_count);
	me->active_mm = &init_mm;
	BUG_ON(me->mm);
	enter_lazy_tlb(&init_mm, me);

	load_sp0(t, &current->thread);
	set_tss_desc(cpu, t);
	load_TR_desc();
	load_LDT(&init_mm.context);

	clear_all_debug_regs();
	dbg_restore_debug_regs();

	fpu__init_cpu();

	if (is_uv_system())
		uv_cpu_init();
}

#else

void cpu_init(void)
{
	int cpu = smp_processor_id();
	struct task_struct *curr = current;
	struct tss_struct *t = &per_cpu(cpu_tss, cpu);
	struct thread_struct *thread = &curr->thread;

	wait_for_master_cpu(cpu);

	/*
	 * Initialize the CR4 shadow before doing anything that could
	 * try to read it.
	 */
	cr4_init_shadow();

	show_ucode_info_early();

	printk(KERN_INFO "Initializing CPU#%d\n", cpu);

	if (cpu_feature_enabled(X86_FEATURE_VME) || cpu_has_tsc || cpu_has_de)
		cr4_clear_bits(X86_CR4_VME|X86_CR4_PVI|X86_CR4_TSD|X86_CR4_DE);

	load_current_idt();
	switch_to_new_gdt(cpu);

	/*
	 * Set up and load the per-CPU TSS and LDT
	 */
	atomic_inc(&init_mm.mm_count);
	curr->active_mm = &init_mm;
	BUG_ON(curr->mm);
	enter_lazy_tlb(&init_mm, curr);

	load_sp0(t, thread);
	set_tss_desc(cpu, t);
	load_TR_desc();
	load_LDT(&init_mm.context);

	t->x86_tss.io_bitmap_base = offsetof(struct tss_struct, io_bitmap);

#ifdef CONFIG_DOUBLEFAULT
	/* Set up doublefault TSS pointer in the GDT */
	__set_tss_desc(cpu, GDT_ENTRY_DOUBLEFAULT_TSS, &doublefault_tss);
#endif

	clear_all_debug_regs();
	dbg_restore_debug_regs();

	fpu__init_cpu();
}
#endif

#ifdef CONFIG_X86_DEBUG_STATIC_CPU_HAS
void warn_pre_alternatives(void)
{
	WARN(1, "You're using static_cpu_has before alternatives have run!\n");
}
EXPORT_SYMBOL_GPL(warn_pre_alternatives);
#endif

inline bool __static_cpu_has_safe(u16 bit)
{
	return boot_cpu_has(bit);
}
EXPORT_SYMBOL_GPL(__static_cpu_has_safe);
{
	int cpu = smp_processor_id();
	struct task_struct *curr = current;
	struct tss_struct *t = &per_cpu(cpu_tss, cpu);
	struct thread_struct *thread = &curr->thread;

	wait_for_master_cpu(cpu);

	/*
	 * Initialize the CR4 shadow before doing anything that could
	 * try to read it.
	 */
	cr4_init_shadow();

	show_ucode_info_early();

	printk(KERN_INFO "Initializing CPU#%d\n", cpu);

	if (cpu_feature_enabled(X86_FEATURE_VME) || cpu_has_tsc || cpu_has_de)
		cr4_clear_bits(X86_CR4_VME|X86_CR4_PVI|X86_CR4_TSD|X86_CR4_DE);

	load_current_idt();
	switch_to_new_gdt(cpu);

	/*
	 * Set up and load the per-CPU TSS and LDT
	 */
	atomic_inc(&init_mm.mm_count);
	curr->active_mm = &init_mm;
	BUG_ON(curr->mm);
	enter_lazy_tlb(&init_mm, curr);

	load_sp0(t, thread);
	set_tss_desc(cpu, t);
	load_TR_desc();
	load_LDT(&init_mm.context);

	t->x86_tss.io_bitmap_base = offsetof(struct tss_struct, io_bitmap);

#ifdef CONFIG_DOUBLEFAULT
	/* Set up doublefault TSS pointer in the GDT */
	__set_tss_desc(cpu, GDT_ENTRY_DOUBLEFAULT_TSS, &doublefault_tss);
#endif

	clear_all_debug_regs();
	dbg_restore_debug_regs();

	fpu__init_cpu();
}
#endif

#ifdef CONFIG_X86_DEBUG_STATIC_CPU_HAS
void warn_pre_alternatives(void)
{
	WARN(1, "You're using static_cpu_has before alternatives have run!\n");
}
EXPORT_SYMBOL_GPL(warn_pre_alternatives);
#endif

inline bool __static_cpu_has_safe(u16 bit)
{
	return boot_cpu_has(bit);
}
EXPORT_SYMBOL_GPL(__static_cpu_has_safe);