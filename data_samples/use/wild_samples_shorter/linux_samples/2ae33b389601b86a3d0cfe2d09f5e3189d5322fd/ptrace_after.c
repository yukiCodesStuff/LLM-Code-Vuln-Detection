
	brk.address = bp_info->addr & ~7UL;
	brk.type = HW_BRK_TYPE_TRANSLATE;
	brk.len = 8;
	if (bp_info->trigger_type & PPC_BREAKPOINT_TRIGGER_READ)
		brk.type |= HW_BRK_TYPE_READ;
	if (bp_info->trigger_type & PPC_BREAKPOINT_TRIGGER_WRITE)
		brk.type |= HW_BRK_TYPE_WRITE;