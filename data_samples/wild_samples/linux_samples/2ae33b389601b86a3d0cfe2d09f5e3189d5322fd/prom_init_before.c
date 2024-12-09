{
}
#else
static void __reloc_toc(void *tocstart, unsigned long offset,
			unsigned long nr_entries)
{
	unsigned long i;
	unsigned long *toc_entry = (unsigned long *)tocstart;

	for (i = 0; i < nr_entries; i++) {
		*toc_entry = *toc_entry + offset;
		toc_entry++;
	}
{
	unsigned long offset = reloc_offset();
	unsigned long nr_entries =
		(__prom_init_toc_end - __prom_init_toc_start) / sizeof(long);

	/* Need to add offset to get at __prom_init_toc_start */
	__reloc_toc(__prom_init_toc_start + offset, offset, nr_entries);

	mb();
}

static void unreloc_toc(void)
{
	unsigned long offset = reloc_offset();
	unsigned long nr_entries =
		(__prom_init_toc_end - __prom_init_toc_start) / sizeof(long);

	mb();

	/* __prom_init_toc_start has been relocated, no need to add offset */
	__reloc_toc(__prom_init_toc_start, -offset, nr_entries);
}
#endif
#endif

/*
 * We enter here early on, when the Open Firmware prom is still
 * handling exceptions and the MMU hash table for us.
 */

unsigned long __init prom_init(unsigned long r3, unsigned long r4,
			       unsigned long pp,
			       unsigned long r6, unsigned long r7,
			       unsigned long kbase)
{	
	unsigned long hdr;

#ifdef CONFIG_PPC32
	unsigned long offset = reloc_offset();
	reloc_got2(offset);
#else
	reloc_toc();
#endif

	/*
	 * First zero the BSS
	 */
	memset(&__bss_start, 0, __bss_stop - __bss_start);

	/*
	 * Init interface to Open Firmware, get some node references,
	 * like /chosen
	 */
	prom_init_client_services(pp);

	/*
	 * See if this OF is old enough that we need to do explicit maps
	 * and other workarounds
	 */
	prom_find_mmu();

	/*
	 * Init prom stdout device
	 */
	prom_init_stdout();

	prom_printf("Preparing to boot %s", linux_banner);

	/*
	 * Get default machine type. At this point, we do not differentiate
	 * between pSeries SMP and pSeries LPAR
	 */
	of_platform = prom_find_machine_type();
	prom_printf("Detected machine type: %x\n", of_platform);

#ifndef CONFIG_NONSTATIC_KERNEL
	/* Bail if this is a kdump kernel. */
	if (PHYSICAL_START > 0)
		prom_panic("Error: You can't boot a kdump kernel from OF!\n");
#endif

	/*
	 * Check for an initrd
	 */
	prom_check_initrd(r3, r4);

#if defined(CONFIG_PPC_PSERIES) || defined(CONFIG_PPC_POWERNV)
	/*
	 * On pSeries, inform the firmware about our capabilities
	 */
	if (of_platform == PLATFORM_PSERIES ||
	    of_platform == PLATFORM_PSERIES_LPAR)
		prom_send_capabilities();
#endif

	/*
	 * Copy the CPU hold code
	 */
	if (of_platform != PLATFORM_POWERMAC)
		copy_and_flush(0, kbase, 0x100, 0);

	/*
	 * Do early parsing of command line
	 */
	early_cmdline_parse();

	/*
	 * Initialize memory management within prom_init
	 */
	prom_init_mem();

	/*
	 * Determine which cpu is actually running right _now_
	 */
	prom_find_boot_cpu();

	/* 
	 * Initialize display devices
	 */
	prom_check_displays();

#ifdef CONFIG_PPC64
	/*
	 * Initialize IOMMU (TCE tables) on pSeries. Do that before anything else
	 * that uses the allocator, we need to make sure we get the top of memory
	 * available for us here...
	 */
	if (of_platform == PLATFORM_PSERIES)
		prom_initialize_tce_table();
#endif

	/*
	 * On non-powermacs, try to instantiate RTAS. PowerMacs don't
	 * have a usable RTAS implementation.
	 */
	if (of_platform != PLATFORM_POWERMAC &&
	    of_platform != PLATFORM_OPAL)
		prom_instantiate_rtas();

#ifdef CONFIG_PPC_POWERNV
	/* Detect HAL and try instanciating it & doing takeover */
	if (of_platform == PLATFORM_PSERIES_LPAR) {
		prom_query_opal();
		if (of_platform == PLATFORM_OPAL) {
			prom_opal_hold_cpus();
			prom_opal_takeover();
		}
	} else if (of_platform == PLATFORM_OPAL)
		prom_instantiate_opal();
#endif

#ifdef CONFIG_PPC64
	/* instantiate sml */
	prom_instantiate_sml();
#endif

	/*
	 * On non-powermacs, put all CPUs in spin-loops.
	 *
	 * PowerMacs use a different mechanism to spin CPUs
	 */
	if (of_platform != PLATFORM_POWERMAC &&
	    of_platform != PLATFORM_OPAL)
		prom_hold_cpus();

	/*
	 * Fill in some infos for use by the kernel later on
	 */
	if (prom_memory_limit)
		prom_setprop(prom.chosen, "/chosen", "linux,memory-limit",
			     &prom_memory_limit,
			     sizeof(prom_memory_limit));
#ifdef CONFIG_PPC64
	if (prom_iommu_off)
		prom_setprop(prom.chosen, "/chosen", "linux,iommu-off",
			     NULL, 0);

	if (prom_iommu_force_on)
		prom_setprop(prom.chosen, "/chosen", "linux,iommu-force-on",
			     NULL, 0);

	if (prom_tce_alloc_start) {
		prom_setprop(prom.chosen, "/chosen", "linux,tce-alloc-start",
			     &prom_tce_alloc_start,
			     sizeof(prom_tce_alloc_start));
		prom_setprop(prom.chosen, "/chosen", "linux,tce-alloc-end",
			     &prom_tce_alloc_end,
			     sizeof(prom_tce_alloc_end));
	}
#endif

	/*
	 * Fixup any known bugs in the device-tree
	 */
	fixup_device_tree();

	/*
	 * Now finally create the flattened device-tree
	 */
	prom_printf("copying OF device tree...\n");
	flatten_device_tree();

	/*
	 * in case stdin is USB and still active on IBM machines...
	 * Unfortunately quiesce crashes on some powermacs if we have
	 * closed stdin already (in particular the powerbook 101). It
	 * appears that the OPAL version of OFW doesn't like it either.
	 */
	if (of_platform != PLATFORM_POWERMAC &&
	    of_platform != PLATFORM_OPAL)
		prom_close_stdin();

	/*
	 * Call OF "quiesce" method to shut down pending DMA's from
	 * devices etc...
	 */
	prom_printf("Calling quiesce...\n");
	call_prom("quiesce", 0, 0);

	/*
	 * And finally, call the kernel passing it the flattened device
	 * tree and NULL as r5, thus triggering the new entry point which
	 * is common to us and kexec
	 */
	hdr = dt_header_start;

	/* Don't print anything after quiesce under OPAL, it crashes OFW */
	if (of_platform != PLATFORM_OPAL) {
		prom_printf("returning from prom_init\n");
		prom_debug("->dt_header_start=0x%x\n", hdr);
	}

#ifdef CONFIG_PPC32
	reloc_got2(-offset);
#else
	unreloc_toc();
#endif

#ifdef CONFIG_PPC_EARLY_DEBUG_OPAL
	/* OPAL early debug gets the OPAL base & entry in r8 and r9 */
	__start(hdr, kbase, 0, 0, 0,
		prom_opal_base, prom_opal_entry);
#else
	__start(hdr, kbase, 0, 0, 0, 0, 0);
#endif

	return 0;
}
{
	unsigned long offset = reloc_offset();
	unsigned long nr_entries =
		(__prom_init_toc_end - __prom_init_toc_start) / sizeof(long);

	mb();

	/* __prom_init_toc_start has been relocated, no need to add offset */
	__reloc_toc(__prom_init_toc_start, -offset, nr_entries);
}
#endif
#endif

/*
 * We enter here early on, when the Open Firmware prom is still
 * handling exceptions and the MMU hash table for us.
 */

unsigned long __init prom_init(unsigned long r3, unsigned long r4,
			       unsigned long pp,
			       unsigned long r6, unsigned long r7,
			       unsigned long kbase)
{	
	unsigned long hdr;

#ifdef CONFIG_PPC32
	unsigned long offset = reloc_offset();
	reloc_got2(offset);
#else
	reloc_toc();
#endif

	/*
	 * First zero the BSS
	 */
	memset(&__bss_start, 0, __bss_stop - __bss_start);

	/*
	 * Init interface to Open Firmware, get some node references,
	 * like /chosen
	 */
	prom_init_client_services(pp);

	/*
	 * See if this OF is old enough that we need to do explicit maps
	 * and other workarounds
	 */
	prom_find_mmu();

	/*
	 * Init prom stdout device
	 */
	prom_init_stdout();

	prom_printf("Preparing to boot %s", linux_banner);

	/*
	 * Get default machine type. At this point, we do not differentiate
	 * between pSeries SMP and pSeries LPAR
	 */
	of_platform = prom_find_machine_type();
	prom_printf("Detected machine type: %x\n", of_platform);

#ifndef CONFIG_NONSTATIC_KERNEL
	/* Bail if this is a kdump kernel. */
	if (PHYSICAL_START > 0)
		prom_panic("Error: You can't boot a kdump kernel from OF!\n");
#endif

	/*
	 * Check for an initrd
	 */
	prom_check_initrd(r3, r4);

#if defined(CONFIG_PPC_PSERIES) || defined(CONFIG_PPC_POWERNV)
	/*
	 * On pSeries, inform the firmware about our capabilities
	 */
	if (of_platform == PLATFORM_PSERIES ||
	    of_platform == PLATFORM_PSERIES_LPAR)
		prom_send_capabilities();
#endif

	/*
	 * Copy the CPU hold code
	 */
	if (of_platform != PLATFORM_POWERMAC)
		copy_and_flush(0, kbase, 0x100, 0);

	/*
	 * Do early parsing of command line
	 */
	early_cmdline_parse();

	/*
	 * Initialize memory management within prom_init
	 */
	prom_init_mem();

	/*
	 * Determine which cpu is actually running right _now_
	 */
	prom_find_boot_cpu();

	/* 
	 * Initialize display devices
	 */
	prom_check_displays();

#ifdef CONFIG_PPC64
	/*
	 * Initialize IOMMU (TCE tables) on pSeries. Do that before anything else
	 * that uses the allocator, we need to make sure we get the top of memory
	 * available for us here...
	 */
	if (of_platform == PLATFORM_PSERIES)
		prom_initialize_tce_table();
#endif

	/*
	 * On non-powermacs, try to instantiate RTAS. PowerMacs don't
	 * have a usable RTAS implementation.
	 */
	if (of_platform != PLATFORM_POWERMAC &&
	    of_platform != PLATFORM_OPAL)
		prom_instantiate_rtas();

#ifdef CONFIG_PPC_POWERNV
	/* Detect HAL and try instanciating it & doing takeover */
	if (of_platform == PLATFORM_PSERIES_LPAR) {
		prom_query_opal();
		if (of_platform == PLATFORM_OPAL) {
			prom_opal_hold_cpus();
			prom_opal_takeover();
		}
	} else if (of_platform == PLATFORM_OPAL)
		prom_instantiate_opal();
#endif

#ifdef CONFIG_PPC64
	/* instantiate sml */
	prom_instantiate_sml();
#endif

	/*
	 * On non-powermacs, put all CPUs in spin-loops.
	 *
	 * PowerMacs use a different mechanism to spin CPUs
	 */
	if (of_platform != PLATFORM_POWERMAC &&
	    of_platform != PLATFORM_OPAL)
		prom_hold_cpus();

	/*
	 * Fill in some infos for use by the kernel later on
	 */
	if (prom_memory_limit)
		prom_setprop(prom.chosen, "/chosen", "linux,memory-limit",
			     &prom_memory_limit,
			     sizeof(prom_memory_limit));
#ifdef CONFIG_PPC64
	if (prom_iommu_off)
		prom_setprop(prom.chosen, "/chosen", "linux,iommu-off",
			     NULL, 0);

	if (prom_iommu_force_on)
		prom_setprop(prom.chosen, "/chosen", "linux,iommu-force-on",
			     NULL, 0);

	if (prom_tce_alloc_start) {
		prom_setprop(prom.chosen, "/chosen", "linux,tce-alloc-start",
			     &prom_tce_alloc_start,
			     sizeof(prom_tce_alloc_start));
		prom_setprop(prom.chosen, "/chosen", "linux,tce-alloc-end",
			     &prom_tce_alloc_end,
			     sizeof(prom_tce_alloc_end));
	}
#endif

	/*
	 * Fixup any known bugs in the device-tree
	 */
	fixup_device_tree();

	/*
	 * Now finally create the flattened device-tree
	 */
	prom_printf("copying OF device tree...\n");
	flatten_device_tree();

	/*
	 * in case stdin is USB and still active on IBM machines...
	 * Unfortunately quiesce crashes on some powermacs if we have
	 * closed stdin already (in particular the powerbook 101). It
	 * appears that the OPAL version of OFW doesn't like it either.
	 */
	if (of_platform != PLATFORM_POWERMAC &&
	    of_platform != PLATFORM_OPAL)
		prom_close_stdin();

	/*
	 * Call OF "quiesce" method to shut down pending DMA's from
	 * devices etc...
	 */
	prom_printf("Calling quiesce...\n");
	call_prom("quiesce", 0, 0);

	/*
	 * And finally, call the kernel passing it the flattened device
	 * tree and NULL as r5, thus triggering the new entry point which
	 * is common to us and kexec
	 */
	hdr = dt_header_start;

	/* Don't print anything after quiesce under OPAL, it crashes OFW */
	if (of_platform != PLATFORM_OPAL) {
		prom_printf("returning from prom_init\n");
		prom_debug("->dt_header_start=0x%x\n", hdr);
	}

#ifdef CONFIG_PPC32
	reloc_got2(-offset);
#else
	unreloc_toc();
#endif

#ifdef CONFIG_PPC_EARLY_DEBUG_OPAL
	/* OPAL early debug gets the OPAL base & entry in r8 and r9 */
	__start(hdr, kbase, 0, 0, 0,
		prom_opal_base, prom_opal_entry);
#else
	__start(hdr, kbase, 0, 0, 0, 0, 0);
#endif

	return 0;
}