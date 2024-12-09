{
	int cpuid;

	/*
	 * If waken up by an INIT in an 82489DX configuration
	 * cpu_callout_mask guarantees we don't get here before
	 * an INIT_deassert IPI reaches our local APIC, so it is
	 * now safe to touch our local APIC.
	 */
	cpuid = smp_processor_id();

	/*
	 * the boot CPU has finished the init stage and is spinning
	 * on callin_map until we finish. We are free to set up this
	 * CPU, first the APIC. (this is probably redundant on most
	 * boards)
	 */
	apic_ap_setup();

	/*
	 * Save our processor parameters. Note: this information
	 * is needed for clock calibration.
	 */
	smp_store_cpu_info(cpuid);

	/*
	 * The topology information must be up to date before
	 * calibrate_delay() and notify_cpu_starting().
	 */
	set_cpu_sibling_map(raw_smp_processor_id());

	/*
	 * Get our bogomips.
	 * Update loops_per_jiffy in cpu_data. Previous call to
	 * smp_store_cpu_info() stored a value that is close but not as
	 * accurate as the value just calculated.
	 */
	calibrate_delay();
	cpu_data(cpuid).loops_per_jiffy = loops_per_jiffy;
	pr_debug("Stack at about %p\n", &cpuid);

	wmb();

	notify_cpu_starting(cpuid);

	/*
	 * Allow the master to continue.
	 */
	cpumask_set_cpu(cpuid, cpu_callin_mask);
}

static int cpu0_logical_apicid;
static int enable_start_cpu0;
/*
 * Activate a secondary processor.
 */
static void notrace start_secondary(void *unused)
{
	/*
	 * Don't put *anything* except direct CPU state initialization
	 * before cpu_init(), SMP booting is too fragile that we want to
	 * limit the things done here to the most necessary things.
	 */
	if (boot_cpu_has(X86_FEATURE_PCID))
		__write_cr4(__read_cr4() | X86_CR4_PCIDE);

#ifdef CONFIG_X86_32
	/* switch away from the initial page table */
	load_cr3(swapper_pg_dir);
	/*
	 * Initialize the CR4 shadow before doing anything that could
	 * try to read it.
	 */
	cr4_init_shadow();
	__flush_tlb_all();
#endif
	load_current_idt();
	cpu_init();
	x86_cpuinit.early_percpu_clock_init();
	preempt_disable();
	smp_callin();

	enable_start_cpu0 = 0;

	/* otherwise gcc will move up smp_processor_id before the cpu_init */
	barrier();
	/*
	 * Check TSC synchronization with the boot CPU:
	 */
	check_tsc_sync_target();

	speculative_store_bypass_ht_init();

	/*
	 * Lock vector_lock, set CPU online and bring the vector
	 * allocator online. Online must be set with vector_lock held
	 * to prevent a concurrent irq setup/teardown from seeing a
	 * half valid vector space.
	 */
	lock_vector_lock();
	set_cpu_online(smp_processor_id(), true);
	lapic_online();
	unlock_vector_lock();
	cpu_set_state_online(smp_processor_id());
	x86_platform.nmi_init();

	/* enable local interrupts */
	local_irq_enable();

	/* to prevent fake stack check failure in clock setup */
	boot_init_stack_canary();

	x86_cpuinit.setup_percpu_clockev();

	wmb();
	cpu_startup_entry(CPUHP_AP_ONLINE_IDLE);
}