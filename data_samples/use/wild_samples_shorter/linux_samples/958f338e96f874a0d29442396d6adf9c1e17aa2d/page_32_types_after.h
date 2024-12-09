#define N_EXCEPTION_STACKS 1

#ifdef CONFIG_X86_PAE
/*
 * This is beyond the 44 bit limit imposed by the 32bit long pfns,
 * but we need the full mask to make sure inverted PROT_NONE
 * entries have all the host bits set in a guest.
 * The real limit is still 44 bits.
 */
#define __PHYSICAL_MASK_SHIFT	52
#define __VIRTUAL_MASK_SHIFT	32

#else  /* !CONFIG_X86_PAE */
#define __PHYSICAL_MASK_SHIFT	32