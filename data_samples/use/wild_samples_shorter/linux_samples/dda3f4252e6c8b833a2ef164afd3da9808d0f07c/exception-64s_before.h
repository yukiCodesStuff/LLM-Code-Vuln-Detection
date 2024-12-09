	nop;								\
	nop

/*
 * r10 must be free to use, r13 must be paca
 */
#define INTERRUPT_TO_KERNEL						\
	STF_ENTRY_BARRIER_SLOT

/*
 * Macros for annotating the expected destination of (h)rfid
 *
	RFSCV;								\
	b	rfscv_flush_fallback

#endif /* __ASSEMBLY__ */

#endif	/* _ASM_POWERPC_EXCEPTION_H */