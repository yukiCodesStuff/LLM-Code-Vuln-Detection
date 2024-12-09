	nop;								\
	nop

#define ENTRY_FLUSH_SLOT						\
	ENTRY_FLUSH_FIXUP_SECTION;					\
	nop;								\
	nop;								\
	nop;

/*
 * r10 must be free to use, r13 must be paca
 */
#define INTERRUPT_TO_KERNEL						\
	STF_ENTRY_BARRIER_SLOT;						\
	ENTRY_FLUSH_SLOT

/*
 * Macros for annotating the expected destination of (h)rfid
 *
	RFSCV;								\
	b	rfscv_flush_fallback

#else /* __ASSEMBLY__ */
/* Prototype for function defined in exceptions-64s.S */
void do_uaccess_flush(void);
#endif /* __ASSEMBLY__ */

#endif	/* _ASM_POWERPC_EXCEPTION_H */