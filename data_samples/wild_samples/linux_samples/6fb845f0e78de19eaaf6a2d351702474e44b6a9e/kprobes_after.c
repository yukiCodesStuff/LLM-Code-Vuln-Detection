{
	if ((addr >= (unsigned long)__kprobes_text_start &&
	    addr < (unsigned long)__kprobes_text_end) ||
	    (addr >= (unsigned long)__entry_text_start &&
	    addr < (unsigned long)__entry_text_end) ||
	    (addr >= (unsigned long)__idmap_text_start &&
	    addr < (unsigned long)__idmap_text_end) ||
	    (addr >= (unsigned long)__hyp_text_start &&
	    addr < (unsigned long)__hyp_text_end) ||
	    !!search_exception_tables(addr))
		return true;

	if (!is_kernel_in_hyp_mode()) {
		if ((addr >= (unsigned long)__hyp_idmap_text_start &&
		    addr < (unsigned long)__hyp_idmap_text_end))
			return true;
	}