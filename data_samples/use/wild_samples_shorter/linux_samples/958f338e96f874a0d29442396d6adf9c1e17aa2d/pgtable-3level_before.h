#endif

/* Encode and de-code a swap entry */
#define MAX_SWAPFILES_CHECK() BUILD_BUG_ON(MAX_SWAPFILES_SHIFT > 5)
#define __swp_type(x)			(((x).val) & 0x1f)
#define __swp_offset(x)			((x).val >> 5)
#define __swp_entry(type, offset)	((swp_entry_t){(type) | (offset) << 5})
#define __pte_to_swp_entry(pte)		((swp_entry_t){ (pte).pte_high })
#define __swp_entry_to_pte(x)		((pte_t){ { .pte_high = (x).val } })

#define gup_get_pte gup_get_pte
/*
 * WARNING: only to be used in the get_user_pages_fast() implementation.
	return pte;
}

#endif /* _ASM_X86_PGTABLE_3LEVEL_H */