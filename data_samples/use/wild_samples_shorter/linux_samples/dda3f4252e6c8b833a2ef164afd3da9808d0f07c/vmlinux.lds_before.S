		__stop___stf_entry_barrier_fixup = .;
	}

	. = ALIGN(8);
	__stf_exit_barrier_fixup : AT(ADDR(__stf_exit_barrier_fixup) - LOAD_OFFSET) {
		__start___stf_exit_barrier_fixup = .;
		*(__stf_exit_barrier_fixup)