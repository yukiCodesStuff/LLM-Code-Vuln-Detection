	do_stf_exit_barrier_fixups(types);
}

void do_rfi_flush_fixups(enum l1d_flush_type types)
{
	unsigned int instrs[3], *dest;
	long *start, *end;