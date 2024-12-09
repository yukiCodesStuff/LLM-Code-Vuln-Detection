 *
 * |     ...            | 11| 10|  9|8|7|6|5| 4| 3|2| 1|0| <- bit number
 * |     ...            |SW3|SW2|SW1|G|L|D|A|CD|WT|U| W|P| <- bit names
 * | TYPE (59-63) | ~OFFSET (9-58)  |0|0|X|X| X| X|X|SD|0| <- swp entry
 *
 * G (8) is aliased and used as a PROT_NONE indicator for
 * !present ptes.  We need to start storing swap entries above
 * there.  We also need to avoid using A and D because of an
 *
 * Bit 7 in swp entry should be 0 because pmd_present checks not only P,
 * but also L and G.
 *
 * The offset is inverted by a binary not operation to make the high
 * physical bits set.
 */
#define SWP_TYPE_BITS		5

#define SWP_OFFSET_FIRST_BIT	(_PAGE_BIT_PROTNONE + 1)

/* We always extract/encode the offset by shifting it all the way up, and then down again */
#define SWP_OFFSET_SHIFT	(SWP_OFFSET_FIRST_BIT+SWP_TYPE_BITS)

#define MAX_SWAPFILES_CHECK() BUILD_BUG_ON(MAX_SWAPFILES_SHIFT > SWP_TYPE_BITS)

/* Extract the high bits for type */
#define __swp_type(x) ((x).val >> (64 - SWP_TYPE_BITS))

/* Shift up (to get rid of type), then down to get value */
#define __swp_offset(x) (~(x).val << SWP_TYPE_BITS >> SWP_OFFSET_SHIFT)

/*
 * Shift the offset up "too far" by TYPE bits, then down again
 * The offset is inverted by a binary not operation to make the high
 * physical bits set.
 */
#define __swp_entry(type, offset) ((swp_entry_t) { \
	(~(unsigned long)(offset) << SWP_OFFSET_SHIFT >> SWP_TYPE_BITS) \
	| ((unsigned long)(type) << (64-SWP_TYPE_BITS)) })

#define __pte_to_swp_entry(pte)		((swp_entry_t) { pte_val((pte)) })
#define __pmd_to_swp_entry(pmd)		((swp_entry_t) { pmd_val((pmd)) })
#define __swp_entry_to_pte(x)		((pte_t) { .pte = (x).val })
#define __swp_entry_to_pmd(x)		((pmd_t) { .pmd = (x).val })
	return true;
}

#include <asm/pgtable-invert.h>

#endif /* !__ASSEMBLY__ */
#endif /* _ASM_X86_PGTABLE_64_H */