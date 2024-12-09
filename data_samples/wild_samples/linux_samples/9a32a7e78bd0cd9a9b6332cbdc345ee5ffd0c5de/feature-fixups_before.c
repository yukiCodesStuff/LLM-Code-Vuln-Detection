{
	do_stf_entry_barrier_fixups(types);
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