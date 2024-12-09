		__stop___stf_entry_barrier_fixup = .;
	}

	. = ALIGN(8);
	__uaccess_flush_fixup : AT(ADDR(__uaccess_flush_fixup) - LOAD_OFFSET) {
		__start___uaccess_flush_fixup = .;
		*(__uaccess_flush_fixup)
		__stop___uaccess_flush_fixup = .;
	}

	. = ALIGN(8);
	__entry_flush_fixup : AT(ADDR(__entry_flush_fixup) - LOAD_OFFSET) {
		__start___entry_flush_fixup = .;
		*(__entry_flush_fixup)