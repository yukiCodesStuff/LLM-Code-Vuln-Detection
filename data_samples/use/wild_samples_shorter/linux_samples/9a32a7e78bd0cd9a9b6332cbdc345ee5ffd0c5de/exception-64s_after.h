	RFSCV;								\
	b	rfscv_flush_fallback

#else /* __ASSEMBLY__ */
/* Prototype for function defined in exceptions-64s.S */
void do_uaccess_flush(void);
#endif /* __ASSEMBLY__ */

#endif	/* _ASM_POWERPC_EXCEPTION_H */