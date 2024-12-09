{
	do_stf_entry_barrier_fixups(types);
	do_stf_exit_barrier_fixups(types);
}

void do_uaccess_flush_fixups(enum l1d_flush_type types)
{
	unsigned int instrs[4], *dest;
	long *start, *end;
	int i;

	start = PTRRELOC(&__start___uaccess_flush_fixup);
	end = PTRRELOC(&__stop___uaccess_flush_fixup);

	instrs[0] = 0x60000000; /* nop */
	instrs[1] = 0x60000000; /* nop */
	instrs[2] = 0x60000000; /* nop */
	instrs[3] = 0x4e800020; /* blr */

	i = 0;
	if (types == L1D_FLUSH_FALLBACK) {
		instrs[3] = 0x60000000; /* nop */
		/* fallthrough to fallback flush */
	}