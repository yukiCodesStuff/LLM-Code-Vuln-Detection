	return false;
}

static inline int pmd_bad(pmd_t pmd)
{
	if (radix_enabled())
		return radix__pmd_bad(pmd);
#define pmd_access_permitted pmd_access_permitted
static inline bool pmd_access_permitted(pmd_t pmd, bool write)
{
	return pte_access_permitted(pmd_pte(pmd), write);
}

#ifdef CONFIG_TRANSPARENT_HUGEPAGE