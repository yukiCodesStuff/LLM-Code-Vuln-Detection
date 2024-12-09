	__stf_entry_barrier_fixup : AT(ADDR(__stf_entry_barrier_fixup) - LOAD_OFFSET) {
		__start___stf_entry_barrier_fixup = .;
		*(__stf_entry_barrier_fixup)
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
		__stop___stf_exit_barrier_fixup = .;
	}

	. = ALIGN(8);
	__rfi_flush_fixup : AT(ADDR(__rfi_flush_fixup) - LOAD_OFFSET) {
		__start___rfi_flush_fixup = .;
		*(__rfi_flush_fixup)
		__stop___rfi_flush_fixup = .;
	}
#endif /* CONFIG_PPC64 */

#ifdef CONFIG_PPC_BARRIER_NOSPEC
	. = ALIGN(8);
	__spec_barrier_fixup : AT(ADDR(__spec_barrier_fixup) - LOAD_OFFSET) {
		__start___barrier_nospec_fixup = .;
		*(__barrier_nospec_fixup)
		__stop___barrier_nospec_fixup = .;
	}
#endif /* CONFIG_PPC_BARRIER_NOSPEC */

#ifdef CONFIG_PPC_FSL_BOOK3E
	. = ALIGN(8);
	__spec_btb_flush_fixup : AT(ADDR(__spec_btb_flush_fixup) - LOAD_OFFSET) {
		__start__btb_flush_fixup = .;
		*(__btb_flush_fixup)
		__stop__btb_flush_fixup = .;
	}
#endif

/*
 * Init sections discarded at runtime
 */
	. = ALIGN(STRICT_ALIGN_SIZE);
	__init_begin = .;
	. = ALIGN(PAGE_SIZE);
	.init.text : AT(ADDR(.init.text) - LOAD_OFFSET) {
		_sinittext = .;
		INIT_TEXT
		_einittext = .;
#ifdef CONFIG_PPC64
		*(.tramp.ftrace.init);
#endif
	} :text

	/* .exit.text is discarded at runtime, not link time,
	 * to deal with references from __bug_table
	 */
	.exit.text : AT(ADDR(.exit.text) - LOAD_OFFSET) {
		EXIT_TEXT
	}