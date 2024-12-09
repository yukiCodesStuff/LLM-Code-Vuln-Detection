#define pte_dirty(pte)		(pte_sw_dirty(pte) || pte_hw_dirty(pte))

#define pte_valid(pte)		(!!(pte_val(pte) & PTE_VALID))
#define pte_valid_not_user(pte) \
	((pte_val(pte) & (PTE_VALID | PTE_USER)) == PTE_VALID)
#define pte_valid_young(pte) \
	((pte_val(pte) & (PTE_VALID | PTE_AF)) == (PTE_VALID | PTE_AF))
#define pte_valid_user(pte) \
	((pte_val(pte) & (PTE_VALID | PTE_USER)) == (PTE_VALID | PTE_USER))

/*
 * p??_access_permitted() is true for valid user mappings (subject to the
 * write permission check). PROT_NONE mappings do not have the PTE_VALID bit
 * set.
 */
#define pte_access_permitted(pte, write) \
	(pte_valid_user(pte) && (!(write) || pte_write(pte)))
#define pmd_access_permitted(pmd, write) \