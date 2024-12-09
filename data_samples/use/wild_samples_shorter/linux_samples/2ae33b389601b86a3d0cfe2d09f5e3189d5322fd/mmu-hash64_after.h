/*
 * VSID allocation (256MB segment)
 *
 * We first generate a 37-bit "proto-VSID". Proto-VSIDs are generated
 * from mmu context id and effective segment id of the address.
 *
 * For user processes max context id is limited to ((1ul << 19) - 5)
 * for kernel space, we use the top 4 context ids to map address as below
 * NOTE: each context only support 64TB now.
 * 0x7fffc -  [ 0xc000000000000000 - 0xc0003fffffffffff ]
 * 0x7fffd -  [ 0xd000000000000000 - 0xd0003fffffffffff ]
 * 0x7fffe -  [ 0xe000000000000000 - 0xe0003fffffffffff ]
 * 0x7ffff -  [ 0xf000000000000000 - 0xf0003fffffffffff ]
 *
 * The proto-VSIDs are then scrambled into real VSIDs with the
 * multiplicative hash:
 *
 * VSID_MULTIPLIER is prime, so in particular it is
 * co-prime to VSID_MODULUS, making this a 1:1 scrambling function.
 * Because the modulus is 2^n-1 we can compute it efficiently without
 * a divide or extra multiply (see below). The scramble function gives
 * robust scattering in the hash table (at least based on some initial
 * results).
 *
 * We also consider VSID 0 special. We use VSID 0 for slb entries mapping
 * bad address. This enables us to consolidate bad address handling in
 * hash_page.
 *
 * We also need to avoid the last segment of the last context, because that
 * would give a protovsid of 0x1fffffffff. That will result in a VSID 0
 * because of the modulo operation in vsid scramble. But the vmemmap
 * (which is what uses region 0xf) will never be close to 64TB in size
 * (it's 56 bytes per page of system memory).
 */

#define CONTEXT_BITS		19
#define ESID_BITS		18
#define ESID_BITS_1T		6

/*
 * 256MB segment
 * The proto-VSID space has 2^(CONTEX_BITS + ESID_BITS) - 1 segments
 * available for user + kernel mapping. The top 4 contexts are used for
 * kernel mapping. Each segment contains 2^28 bytes. Each
 * context maps 2^46 bytes (64TB) so we can support 2^19-1 contexts
 * (19 == 37 + 28 - 46).
 */
#define MAX_USER_CONTEXT	((ASM_CONST(1) << CONTEXT_BITS) - 5)

/*
 * This should be computed such that protovosid * vsid_mulitplier
 * doesn't overflow 64 bits. It should also be co-prime to vsid_modulus
 */
#define VSID_MULTIPLIER_256M	ASM_CONST(12538073)	/* 24-bit prime */
#define VSID_BITS_256M		(CONTEXT_BITS + ESID_BITS)
#define VSID_MODULUS_256M	((1UL<<VSID_BITS_256M)-1)

#define VSID_MULTIPLIER_1T	ASM_CONST(12538073)	/* 24-bit prime */
#define VSID_BITS_1T		(CONTEXT_BITS + ESID_BITS_1T)
#define VSID_MODULUS_1T		((1UL<<VSID_BITS_1T)-1)


#define USER_VSID_RANGE	(1UL << (ESID_BITS + SID_SHIFT))

/*
 * This macro generates asm code to compute the VSID scramble
 * function.  Used in slb_allocate() and do_stab_bolted.  The function
	srdi	rx,rt,VSID_BITS_##size;					\
	clrldi	rt,rt,(64-VSID_BITS_##size);				\
	add	rt,rt,rx;		/* add high and low bits */	\
	/* NOTE: explanation based on VSID_BITS_##size = 36		\
	 * Now, r3 == VSID (mod 2^36-1), and lies between 0 and		\
	 * 2^36-1+2^28-1.  That in particular means that if r3 >=	\
	 * 2^36-1, then r3+1 has the 2^36 bit set.  So, if r3+1 has	\
	 * the bit clear, r3 already has the answer we want, if it	\
	 * doesn't, the answer is the low 36 bits of r3+1.  So in all	\
	})
#endif /* 1 */

/* Returns the segment size indicator for a user address */
static inline int user_segment_size(unsigned long addr)
{
	/* Use 1T segments if possible for addresses >= 1T */
	return MMU_SEGSIZE_256M;
}

static inline unsigned long get_vsid(unsigned long context, unsigned long ea,
				     int ssize)
{
	/*
	 * Bad address. We return VSID 0 for that
	 */
	if ((ea & ~REGION_MASK) >= PGTABLE_RANGE)
		return 0;

	if (ssize == MMU_SEGSIZE_256M)
		return vsid_scramble((context << ESID_BITS)
				     | (ea >> SID_SHIFT), 256M);
	return vsid_scramble((context << ESID_BITS_1T)
			     | (ea >> SID_SHIFT_1T), 1T);
}

/*
 * This is only valid for addresses >= PAGE_OFFSET
 *
 * For kernel space, we use the top 4 context ids to map address as below
 * 0x7fffc -  [ 0xc000000000000000 - 0xc0003fffffffffff ]
 * 0x7fffd -  [ 0xd000000000000000 - 0xd0003fffffffffff ]
 * 0x7fffe -  [ 0xe000000000000000 - 0xe0003fffffffffff ]
 * 0x7ffff -  [ 0xf000000000000000 - 0xf0003fffffffffff ]
 */
static inline unsigned long get_kernel_vsid(unsigned long ea, int ssize)
{
	unsigned long context;

	/*
	 * kernel take the top 4 context from the available range
	 */
	context = (MAX_USER_CONTEXT) + ((ea >> 60) - 0xc) + 1;
	return get_vsid(context, ea, ssize);
}
#endif /* __ASSEMBLY__ */

#endif /* _ASM_POWERPC_MMU_HASH64_H_ */