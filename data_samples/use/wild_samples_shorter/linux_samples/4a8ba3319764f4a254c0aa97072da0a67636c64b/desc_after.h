		gdt[GDT_ENTRY_TLS_MIN + i] = t->tls_array[i];
}

/* This intentionally ignores lm, since 32-bit apps don't have that field. */
#define LDT_empty(info)					\
	((info)->base_addr		== 0	&&	\
	 (info)->limit			== 0	&&	\
	 (info)->contents		== 0	&&	\
	 (info)->read_exec_only		== 1	&&	\
	 (info)->seg_not_present	== 1	&&	\
	 (info)->useable		== 0)

/* Lots of programs expect an all-zero user_desc to mean "no segment at all". */
static inline bool LDT_zero(const struct user_desc *info)
{
	return (info->base_addr		== 0 &&
		info->limit		== 0 &&
		info->contents		== 0 &&
		info->read_exec_only	== 0 &&
		info->seg_32bit		== 0 &&
		info->limit_in_pages	== 0 &&
		info->seg_not_present	== 0 &&
		info->useable		== 0);
}

static inline void clear_LDT(void)
{
	set_ldt(NULL, 0);