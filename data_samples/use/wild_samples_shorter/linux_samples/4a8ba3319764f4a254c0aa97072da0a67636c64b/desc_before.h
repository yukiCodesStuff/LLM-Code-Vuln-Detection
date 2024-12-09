		gdt[GDT_ENTRY_TLS_MIN + i] = t->tls_array[i];
}

#define _LDT_empty(info)				\
	((info)->base_addr		== 0	&&	\
	 (info)->limit			== 0	&&	\
	 (info)->contents		== 0	&&	\
	 (info)->read_exec_only		== 1	&&	\
	 (info)->seg_not_present	== 1	&&	\
	 (info)->useable		== 0)

#ifdef CONFIG_X86_64
#define LDT_empty(info) (_LDT_empty(info) && ((info)->lm == 0))
#else
#define LDT_empty(info) (_LDT_empty(info))
#endif

static inline void clear_LDT(void)
{
	set_ldt(NULL, 0);