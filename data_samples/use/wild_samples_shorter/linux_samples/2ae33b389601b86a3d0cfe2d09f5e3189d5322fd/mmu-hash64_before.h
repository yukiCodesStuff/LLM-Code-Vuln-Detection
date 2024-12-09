/*
 * VSID allocation (256MB segment)
 *
 * We first generate a 38-bit "proto-VSID".  For kernel addresses this
 * is equal to the ESID | 1 << 37, for user addresses it is:
 *	(context << USER_ESID_BITS) | (esid & ((1U << USER_ESID_BITS) - 1)
 *
 * This splits the proto-VSID into the below range
 *  0 - (2^(CONTEXT_BITS + USER_ESID_BITS) - 1) : User proto-VSID range
 *  2^(CONTEXT_BITS + USER_ESID_BITS) - 2^(VSID_BITS) : Kernel proto-VSID range
 *
 * We also have CONTEXT_BITS + USER_ESID_BITS = VSID_BITS - 1
 * That is, we assign half of the space to user processes and half
 * to the kernel.
 *
 * The proto-VSIDs are then scrambled into real VSIDs with the
 * multiplicative hash:
 *
 * VSID_MULTIPLIER is prime, so in particular it is
 * co-prime to VSID_MODULUS, making this a 1:1 scrambling function.
 * Because the modulus is 2^n-1 we can compute it efficiently without
 * a divide or extra multiply (see below).
 *
 * This scheme has several advantages over older methods:
 *
 *	- We have VSIDs allocated for every kernel address
 * (i.e. everything above 0xC000000000000000), except the very top
 * segment, which simplifies several things.
 *
 *	- We allow for USER_ESID_BITS significant bits of ESID and
 * CONTEXT_BITS  bits of context for user addresses.
 *  i.e. 64T (46 bits) of address space for up to half a million contexts.
 *
 *	- The scramble function gives robust scattering in the hash
 * table (at least based on some initial results).  The previous
 * method was more susceptible to pathological cases giving excessive
 * hash collisions.
 */

/*
 * This should be computed such that protovosid * vsid_mulitplier
 * doesn't overflow 64 bits. It should also be co-prime to vsid_modulus
 */
#define VSID_MULTIPLIER_256M	ASM_CONST(12538073)	/* 24-bit prime */
#define VSID_BITS_256M		38
#define VSID_MODULUS_256M	((1UL<<VSID_BITS_256M)-1)

#define VSID_MULTIPLIER_1T	ASM_CONST(12538073)	/* 24-bit prime */
#define VSID_BITS_1T		26
#define VSID_MODULUS_1T		((1UL<<VSID_BITS_1T)-1)

#define CONTEXT_BITS		19
#define USER_ESID_BITS		18
#define USER_ESID_BITS_1T	6

#define USER_VSID_RANGE	(1UL << (USER_ESID_BITS + SID_SHIFT))

/*
 * This macro generates asm code to compute the VSID scramble
 * function.  Used in slb_allocate() and do_stab_bolted.  The function
	srdi	rx,rt,VSID_BITS_##size;					\
	clrldi	rt,rt,(64-VSID_BITS_##size);				\
	add	rt,rt,rx;		/* add high and low bits */	\
	/* Now, r3 == VSID (mod 2^36-1), and lies between 0 and		\
	 * 2^36-1+2^28-1.  That in particular means that if r3 >=	\
	 * 2^36-1, then r3+1 has the 2^36 bit set.  So, if r3+1 has	\
	 * the bit clear, r3 already has the answer we want, if it	\
	 * doesn't, the answer is the low 36 bits of r3+1.  So in all	\
	})
#endif /* 1 */

/*
 * This is only valid for addresses >= PAGE_OFFSET
 * The proto-VSID space is divided into two class
 * User:   0 to 2^(CONTEXT_BITS + USER_ESID_BITS) -1
 * kernel: 2^(CONTEXT_BITS + USER_ESID_BITS) to 2^(VSID_BITS) - 1
 *
 * With KERNEL_START at 0xc000000000000000, the proto vsid for
 * the kernel ends up with 0xc00000000 (36 bits). With 64TB
 * support we need to have kernel proto-VSID in the
 * [2^37 to 2^38 - 1] range due to the increased USER_ESID_BITS.
 */
static inline unsigned long get_kernel_vsid(unsigned long ea, int ssize)
{
	unsigned long proto_vsid;
	/*
	 * We need to make sure proto_vsid for the kernel is
	 * >= 2^(CONTEXT_BITS + USER_ESID_BITS[_1T])
	 */
	if (ssize == MMU_SEGSIZE_256M) {
		proto_vsid = ea >> SID_SHIFT;
		proto_vsid |= (1UL << (CONTEXT_BITS + USER_ESID_BITS));
		return vsid_scramble(proto_vsid, 256M);
	}
	proto_vsid = ea >> SID_SHIFT_1T;
	proto_vsid |= (1UL << (CONTEXT_BITS + USER_ESID_BITS_1T));
	return vsid_scramble(proto_vsid, 1T);
}

/* Returns the segment size indicator for a user address */
static inline int user_segment_size(unsigned long addr)
{
	/* Use 1T segments if possible for addresses >= 1T */
	return MMU_SEGSIZE_256M;
}

/* This is only valid for user addresses (which are below 2^44) */
static inline unsigned long get_vsid(unsigned long context, unsigned long ea,
				     int ssize)
{
	if (ssize == MMU_SEGSIZE_256M)
		return vsid_scramble((context << USER_ESID_BITS)
				     | (ea >> SID_SHIFT), 256M);
	return vsid_scramble((context << USER_ESID_BITS_1T)
			     | (ea >> SID_SHIFT_1T), 1T);
}

#endif /* __ASSEMBLY__ */

#endif /* _ASM_POWERPC_MMU_HASH64_H_ */