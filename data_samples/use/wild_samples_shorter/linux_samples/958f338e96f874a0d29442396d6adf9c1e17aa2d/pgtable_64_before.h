 *
 * |     ...            | 11| 10|  9|8|7|6|5| 4| 3|2| 1|0| <- bit number
 * |     ...            |SW3|SW2|SW1|G|L|D|A|CD|WT|U| W|P| <- bit names
 * | OFFSET (14->63) | TYPE (9-13)  |0|0|X|X| X| X|X|SD|0| <- swp entry
 *
 * G (8) is aliased and used as a PROT_NONE indicator for
 * !present ptes.  We need to start storing swap entries above
 * there.  We also need to avoid using A and D because of an
 *
 * Bit 7 in swp entry should be 0 because pmd_present checks not only P,
 * but also L and G.
 */
#define SWP_TYPE_FIRST_BIT (_PAGE_BIT_PROTNONE + 1)
#define SWP_TYPE_BITS 5
/* Place the offset above the type: */
#define SWP_OFFSET_FIRST_BIT (SWP_TYPE_FIRST_BIT + SWP_TYPE_BITS)

#define MAX_SWAPFILES_CHECK() BUILD_BUG_ON(MAX_SWAPFILES_SHIFT > SWP_TYPE_BITS)

#define __swp_type(x)			(((x).val >> (SWP_TYPE_FIRST_BIT)) \
					 & ((1U << SWP_TYPE_BITS) - 1))
#define __swp_offset(x)			((x).val >> SWP_OFFSET_FIRST_BIT)
#define __swp_entry(type, offset)	((swp_entry_t) { \
					 ((type) << (SWP_TYPE_FIRST_BIT)) \
					 | ((offset) << SWP_OFFSET_FIRST_BIT) })
#define __pte_to_swp_entry(pte)		((swp_entry_t) { pte_val((pte)) })
#define __pmd_to_swp_entry(pmd)		((swp_entry_t) { pmd_val((pmd)) })
#define __swp_entry_to_pte(x)		((pte_t) { .pte = (x).val })
#define __swp_entry_to_pmd(x)		((pmd_t) { .pmd = (x).val })
	return true;
}

#endif /* !__ASSEMBLY__ */
#endif /* _ASM_X86_PGTABLE_64_H */