	if (mode != not_refill && check_for_high_segbits) {
		uasm_l_large_segbits_fault(l, *p);
		/*
		 * We get here if we are an xsseg address, or if we are
		 * an xuseg address above (PGDIR_SHIFT+PGDIR_BITS) boundary.
		 *
		 * Ignoring xsseg (assume disabled so would generate
		 * (address errors?), the only remaining possibility
		 * is the upper xuseg addresses.  On processors with
		 * TLB_SEGBITS <= PGDIR_SHIFT+PGDIR_BITS, these
		 * addresses would have taken an address error. We try
		 * to mimic that here by taking a load/istream page
		 * fault.
		 */
		UASM_i_LA(p, ptr, (unsigned long)tlb_do_page_fault_0);
		uasm_i_jr(p, ptr);

		if (mode == refill_scratch) {
			if (scratch_reg >= 0)
				UASM_i_MFC0(p, 1, c0_kscratch(), scratch_reg);
			else
				UASM_i_LW(p, 1, scratchpad_offset(0), 0);
		} else {
			uasm_i_nop(p);
		}
	}
{
#ifdef CONFIG_SMP
# ifdef CONFIG_PHYS_ADDR_T_64BIT
	if (cpu_has_64bits)
		uasm_i_lld(p, pte, 0, ptr);
	else
# endif
		UASM_i_LL(p, pte, 0, ptr);
#else
# ifdef CONFIG_PHYS_ADDR_T_64BIT
	if (cpu_has_64bits)
		uasm_i_ld(p, pte, 0, ptr);
	else
# endif
		UASM_i_LW(p, pte, 0, ptr);
#endif
}
		} else {
			uasm_i_andi(&p, wr.r3, wr.r3, 2);
			uasm_il_beqz(&p, &r, wr.r3, label_tlbl_goaround2);
		}
		if (PM_DEFAULT_MASK == 0)
			uasm_i_nop(&p);
		/*
		 * We clobbered C0_PAGEMASK, restore it.  On the other branch
		 * it is restored in build_huge_tlb_write_entry.
		 */
		build_restore_pagemask(&p, &r, wr.r3, label_nopage_tlbl, 0);

		uasm_l_tlbl_goaround2(&l, p);
	}
	uasm_i_ori(&p, wr.r1, wr.r1, (_PAGE_ACCESSED | _PAGE_VALID));
	build_huge_handler_tail(&p, &r, &l, wr.r1, wr.r2, 1);
#endif

	uasm_l_nopage_tlbl(&l, p);
	build_restore_work_registers(&p);
#ifdef CONFIG_CPU_MICROMIPS
	if ((unsigned long)tlb_do_page_fault_0 & 1) {
		uasm_i_lui(&p, K0, uasm_rel_hi((long)tlb_do_page_fault_0));
		uasm_i_addiu(&p, K0, K0, uasm_rel_lo((long)tlb_do_page_fault_0));
		uasm_i_jr(&p, K0);
	} else
#endif
	uasm_i_j(&p, (unsigned long)tlb_do_page_fault_0 & 0x0fffffff);
	uasm_i_nop(&p);

	if (p >= (u32 *)handle_tlbl_end)
		panic("TLB load handler fastpath space exceeded");

	uasm_resolve_relocs(relocs, labels);
	pr_debug("Wrote TLB load handler fastpath (%u instructions).\n",
		 (unsigned int)(p - (u32 *)handle_tlbl));

	dump_handler("r4000_tlb_load", handle_tlbl, handle_tlbl_end);
}
{
	u32 *p = (u32 *)msk_isa16_mode((ulong)handle_tlbs);
	struct uasm_label *l = labels;
	struct uasm_reloc *r = relocs;
	struct work_registers wr;

	memset(p, 0, handle_tlbs_end - (char *)p);
	memset(labels, 0, sizeof(labels));
	memset(relocs, 0, sizeof(relocs));

	wr = build_r4000_tlbchange_handler_head(&p, &l, &r);
	build_pte_writable(&p, &r, wr.r1, wr.r2, wr.r3, label_nopage_tlbs);
	if (m4kc_tlbp_war())
		build_tlb_probe_entry(&p);
	build_make_write(&p, &r, wr.r1, wr.r2, wr.r3);
	build_r4000_tlbchange_handler_tail(&p, &l, &r, wr.r1, wr.r2);

#ifdef CONFIG_MIPS_HUGE_TLB_SUPPORT
	/*
	 * This is the entry point when
	 * build_r4000_tlbchange_handler_head spots a huge page.
	 */
	uasm_l_tlb_huge_update(&l, p);
	iPTE_LW(&p, wr.r1, wr.r2);
	build_pte_writable(&p, &r, wr.r1, wr.r2, wr.r3, label_nopage_tlbs);
	build_tlb_probe_entry(&p);
	uasm_i_ori(&p, wr.r1, wr.r1,
		   _PAGE_ACCESSED | _PAGE_MODIFIED | _PAGE_VALID | _PAGE_DIRTY);
	build_huge_handler_tail(&p, &r, &l, wr.r1, wr.r2, 1);
#endif

	uasm_l_nopage_tlbs(&l, p);
	build_restore_work_registers(&p);
#ifdef CONFIG_CPU_MICROMIPS
	if ((unsigned long)tlb_do_page_fault_1 & 1) {
		uasm_i_lui(&p, K0, uasm_rel_hi((long)tlb_do_page_fault_1));
		uasm_i_addiu(&p, K0, K0, uasm_rel_lo((long)tlb_do_page_fault_1));
		uasm_i_jr(&p, K0);
	} else
#endif
	uasm_i_j(&p, (unsigned long)tlb_do_page_fault_1 & 0x0fffffff);
	uasm_i_nop(&p);

	if (p >= (u32 *)handle_tlbs_end)
		panic("TLB store handler fastpath space exceeded");

	uasm_resolve_relocs(relocs, labels);
	pr_debug("Wrote TLB store handler fastpath (%u instructions).\n",
		 (unsigned int)(p - (u32 *)handle_tlbs));

	dump_handler("r4000_tlb_store", handle_tlbs, handle_tlbs_end);
}
{
	u32 *p = (u32 *)msk_isa16_mode((ulong)handle_tlbm);
	struct uasm_label *l = labels;
	struct uasm_reloc *r = relocs;
	struct work_registers wr;

	memset(p, 0, handle_tlbm_end - (char *)p);
	memset(labels, 0, sizeof(labels));
	memset(relocs, 0, sizeof(relocs));

	wr = build_r4000_tlbchange_handler_head(&p, &l, &r);
	build_pte_modifiable(&p, &r, wr.r1, wr.r2, wr.r3, label_nopage_tlbm);
	if (m4kc_tlbp_war())
		build_tlb_probe_entry(&p);
	/* Present and writable bits set, set accessed and dirty bits. */
	build_make_write(&p, &r, wr.r1, wr.r2, wr.r3);
	build_r4000_tlbchange_handler_tail(&p, &l, &r, wr.r1, wr.r2);

#ifdef CONFIG_MIPS_HUGE_TLB_SUPPORT
	/*
	 * This is the entry point when
	 * build_r4000_tlbchange_handler_head spots a huge page.
	 */
	uasm_l_tlb_huge_update(&l, p);
	iPTE_LW(&p, wr.r1, wr.r2);
	build_pte_modifiable(&p, &r, wr.r1, wr.r2,  wr.r3, label_nopage_tlbm);
	build_tlb_probe_entry(&p);
	uasm_i_ori(&p, wr.r1, wr.r1,
		   _PAGE_ACCESSED | _PAGE_MODIFIED | _PAGE_VALID | _PAGE_DIRTY);
	build_huge_handler_tail(&p, &r, &l, wr.r1, wr.r2, 0);
#endif

	uasm_l_nopage_tlbm(&l, p);
	build_restore_work_registers(&p);
#ifdef CONFIG_CPU_MICROMIPS
	if ((unsigned long)tlb_do_page_fault_1 & 1) {
		uasm_i_lui(&p, K0, uasm_rel_hi((long)tlb_do_page_fault_1));
		uasm_i_addiu(&p, K0, K0, uasm_rel_lo((long)tlb_do_page_fault_1));
		uasm_i_jr(&p, K0);
	} else
#endif
	uasm_i_j(&p, (unsigned long)tlb_do_page_fault_1 & 0x0fffffff);
	uasm_i_nop(&p);

	if (p >= (u32 *)handle_tlbm_end)
		panic("TLB modify handler fastpath space exceeded");

	uasm_resolve_relocs(relocs, labels);
	pr_debug("Wrote TLB modify handler fastpath (%u instructions).\n",
		 (unsigned int)(p - (u32 *)handle_tlbm));

	dump_handler("r4000_tlb_modify", handle_tlbm, handle_tlbm_end);
}