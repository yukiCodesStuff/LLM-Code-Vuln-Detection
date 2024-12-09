#endif

#ifdef CONFIG_ARC_HAS_ACCL_REGS
	ST2	r58, r59, PT_sp + 12
#endif

.endm


	LD2	gp, fp, PT_r26		; gp (r26), fp (r27)

	ld	r12, [sp, PT_sp + 4]
	ld	r30, [sp, PT_sp + 8]

	; Restore SP (into AUX_USER_SP) only if returning to U mode
	;  - for K mode, it will be implicitly restored as stack is unwound
	;  - Z flag set on K is inverse of what hardware does on interrupt entry
#endif

#ifdef CONFIG_ARC_HAS_ACCL_REGS
	LD2	r58, r59, PT_sp + 12
#endif
.endm

/*------------------------------------------------------------------------*/