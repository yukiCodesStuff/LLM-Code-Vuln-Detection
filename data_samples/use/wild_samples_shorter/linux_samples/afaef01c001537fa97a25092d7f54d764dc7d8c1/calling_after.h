
#endif

.macro STACKLEAK_ERASE_NOCLOBBER
#ifdef CONFIG_GCC_PLUGIN_STACKLEAK
	PUSH_AND_CLEAR_REGS
	call stackleak_erase
	POP_REGS
#endif
.endm

#endif /* CONFIG_X86_64 */

.macro STACKLEAK_ERASE
#ifdef CONFIG_GCC_PLUGIN_STACKLEAK
	call stackleak_erase
#endif
.endm

/*
 * This does 'call enter_from_user_mode' unless we can avoid it based on
 * kernel config or using the static jump infrastructure.
 */