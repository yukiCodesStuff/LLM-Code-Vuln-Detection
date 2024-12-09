static inline void init_espfix_bsp(void) { }
#endif

#ifndef __HAVE_ARCH_PFN_MODIFY_ALLOWED
static inline bool pfn_modify_allowed(unsigned long pfn, pgprot_t prot)
{
	return true;
}

static inline bool arch_has_pfn_modify_check(void)
{
	return false;
}
#endif /* !_HAVE_ARCH_PFN_MODIFY_ALLOWED */

#endif /* !__ASSEMBLY__ */

#ifndef io_remap_pfn_range
#define io_remap_pfn_range remap_pfn_range