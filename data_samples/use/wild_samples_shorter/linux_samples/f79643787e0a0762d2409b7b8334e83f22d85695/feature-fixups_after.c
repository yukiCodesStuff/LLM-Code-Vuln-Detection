	do_stf_exit_barrier_fixups(types);
}

void do_entry_flush_fixups(enum l1d_flush_type types)
{
	unsigned int instrs[3], *dest;
	long *start, *end;
	int i;

	start = PTRRELOC(&__start___entry_flush_fixup);
	end = PTRRELOC(&__stop___entry_flush_fixup);

	instrs[0] = 0x60000000; /* nop */
	instrs[1] = 0x60000000; /* nop */
	instrs[2] = 0x60000000; /* nop */

	i = 0;
	if (types == L1D_FLUSH_FALLBACK) {
		instrs[i++] = 0x7d4802a6; /* mflr r10		*/
		instrs[i++] = 0x60000000; /* branch patched below */
		instrs[i++] = 0x7d4803a6; /* mtlr r10		*/
	}

	if (types & L1D_FLUSH_ORI) {
		instrs[i++] = 0x63ff0000; /* ori 31,31,0 speculation barrier */
		instrs[i++] = 0x63de0000; /* ori 30,30,0 L1d flush*/
	}

	if (types & L1D_FLUSH_MTTRIG)
		instrs[i++] = 0x7c12dba6; /* mtspr TRIG2,r0 (SPR #882) */

	for (i = 0; start < end; start++, i++) {
		dest = (void *)start + *start;

		pr_devel("patching dest %lx\n", (unsigned long)dest);

		patch_instruction((struct ppc_inst *)dest, ppc_inst(instrs[0]));

		if (types == L1D_FLUSH_FALLBACK)
			patch_branch((struct ppc_inst *)(dest + 1), (unsigned long)&entry_flush_fallback,
				     BRANCH_SET_LINK);
		else
			patch_instruction((struct ppc_inst *)(dest + 1), ppc_inst(instrs[1]));

		patch_instruction((struct ppc_inst *)(dest + 2), ppc_inst(instrs[2]));
	}

	printk(KERN_DEBUG "entry-flush: patched %d locations (%s flush)\n", i,
		(types == L1D_FLUSH_NONE)       ? "no" :
		(types == L1D_FLUSH_FALLBACK)   ? "fallback displacement" :
		(types &  L1D_FLUSH_ORI)        ? (types & L1D_FLUSH_MTTRIG)
							? "ori+mttrig type"
							: "ori type" :
		(types &  L1D_FLUSH_MTTRIG)     ? "mttrig type"
						: "unknown");
}

void do_rfi_flush_fixups(enum l1d_flush_type types)
{
	unsigned int instrs[3], *dest;
	long *start, *end;