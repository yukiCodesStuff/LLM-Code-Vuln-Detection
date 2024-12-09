		__stop___stf_entry_barrier_fixup = .;
	}

	. = ALIGN(8);
	__entry_flush_fixup : AT(ADDR(__entry_flush_fixup) - LOAD_OFFSET) {
		__start___entry_flush_fixup = .;
		*(__entry_flush_fixup)
		__stop___entry_flush_fixup = .;
	}

	. = ALIGN(8);
	__stf_exit_barrier_fixup : AT(ADDR(__stf_exit_barrier_fixup) - LOAD_OFFSET) {
		__start___stf_exit_barrier_fixup = .;
		*(__stf_exit_barrier_fixup)