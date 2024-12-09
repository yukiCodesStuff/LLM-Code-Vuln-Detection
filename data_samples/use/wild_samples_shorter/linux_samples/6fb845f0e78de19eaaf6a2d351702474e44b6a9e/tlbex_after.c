		 * to mimic that here by taking a load/istream page
		 * fault.
		 */
		if (IS_ENABLED(CONFIG_CPU_LOONGSON3_WORKAROUNDS))
			uasm_i_sync(p, 0);
		UASM_i_LA(p, ptr, (unsigned long)tlb_do_page_fault_0);
		uasm_i_jr(p, ptr);

		if (mode == refill_scratch) {
iPTE_LW(u32 **p, unsigned int pte, unsigned int ptr)
{
#ifdef CONFIG_SMP
	if (IS_ENABLED(CONFIG_CPU_LOONGSON3_WORKAROUNDS))
		uasm_i_sync(p, 0);
# ifdef CONFIG_PHYS_ADDR_T_64BIT
	if (cpu_has_64bits)
		uasm_i_lld(p, pte, 0, ptr);
	else
#endif

	uasm_l_nopage_tlbl(&l, p);
	if (IS_ENABLED(CONFIG_CPU_LOONGSON3_WORKAROUNDS))
		uasm_i_sync(&p, 0);
	build_restore_work_registers(&p);
#ifdef CONFIG_CPU_MICROMIPS
	if ((unsigned long)tlb_do_page_fault_0 & 1) {
		uasm_i_lui(&p, K0, uasm_rel_hi((long)tlb_do_page_fault_0));
#endif

	uasm_l_nopage_tlbs(&l, p);
	if (IS_ENABLED(CONFIG_CPU_LOONGSON3_WORKAROUNDS))
		uasm_i_sync(&p, 0);
	build_restore_work_registers(&p);
#ifdef CONFIG_CPU_MICROMIPS
	if ((unsigned long)tlb_do_page_fault_1 & 1) {
		uasm_i_lui(&p, K0, uasm_rel_hi((long)tlb_do_page_fault_1));
#endif

	uasm_l_nopage_tlbm(&l, p);
	if (IS_ENABLED(CONFIG_CPU_LOONGSON3_WORKAROUNDS))
		uasm_i_sync(&p, 0);
	build_restore_work_registers(&p);
#ifdef CONFIG_CPU_MICROMIPS
	if ((unsigned long)tlb_do_page_fault_1 & 1) {
		uasm_i_lui(&p, K0, uasm_rel_hi((long)tlb_do_page_fault_1));