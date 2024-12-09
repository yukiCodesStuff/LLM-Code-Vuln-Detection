	} else {
		mask = (1ul << (SID_SHIFT_1T - VPN_SHIFT)) - 1;
		vsid = vpn >> (SID_SHIFT_1T - VPN_SHIFT);
		hash = vsid ^ (vsid << 25) ^
			((vpn & mask) >> (shift - VPN_SHIFT)) ;
	}
	return hash & 0x7fffffffffUL;
}

extern int __hash_page_4K(unsigned long ea, unsigned long access,
			  unsigned long vsid, pte_t *ptep, unsigned long trap,
			  unsigned int local, int ssize, int subpage_prot);
extern int __hash_page_64K(unsigned long ea, unsigned long access,
			   unsigned long vsid, pte_t *ptep, unsigned long trap,
			   unsigned int local, int ssize);
struct mm_struct;
unsigned int hash_page_do_lazy_icache(unsigned int pp, pte_t pte, int trap);
extern int hash_page(unsigned long ea, unsigned long access, unsigned long trap);
int __hash_page_huge(unsigned long ea, unsigned long access, unsigned long vsid,
		     pte_t *ptep, unsigned long trap, int local, int ssize,
		     unsigned int shift, unsigned int mmu_psize);
extern void hash_failure_debug(unsigned long ea, unsigned long access,
			       unsigned long vsid, unsigned long trap,
			       int ssize, int psize, unsigned long pte);
extern int htab_bolt_mapping(unsigned long vstart, unsigned long vend,
			     unsigned long pstart, unsigned long prot,
			     int psize, int ssize);
extern void add_gpage(u64 addr, u64 page_size, unsigned long number_of_pages);
extern void demote_segment_4k(struct mm_struct *mm, unsigned long addr);

extern void hpte_init_native(void);
extern void hpte_init_lpar(void);
extern void hpte_init_beat(void);
extern void hpte_init_beat_v3(void);

extern void stabs_alloc(void);
extern void slb_initialize(void);
extern void slb_flush_and_rebolt(void);
extern void stab_initialize(unsigned long stab);

extern void slb_vmalloc_update(void);
extern void slb_set_size(u16 size);
#endif /* __ASSEMBLY__ */

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
 *	VSID = (proto-VSID * VSID_MULTIPLIER) % VSID_MODULUS
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
 * computed is: (protovsid*VSID_MULTIPLIER) % VSID_MODULUS
 *
 *	rt = register continaing the proto-VSID and into which the
 *		VSID will be stored
 *	rx = scratch register (clobbered)
 *
 * 	- rt and rx must be different registers
 * 	- The answer will end up in the low VSID_BITS bits of rt.  The higher
 * 	  bits may contain other garbage, so you may need to mask the
 * 	  result.
 */
#define ASM_VSID_SCRAMBLE(rt, rx, size)					\
	lis	rx,VSID_MULTIPLIER_##size@h;				\
	ori	rx,rx,VSID_MULTIPLIER_##size@l;				\
	mulld	rt,rt,rx;		/* rt = rt * MULTIPLIER */	\
									\
	srdi	rx,rt,VSID_BITS_##size;					\
	clrldi	rt,rt,(64-VSID_BITS_##size);				\
	add	rt,rt,rx;		/* add high and low bits */	\
	/* NOTE: explanation based on VSID_BITS_##size = 36		\
	 * Now, r3 == VSID (mod 2^36-1), and lies between 0 and		\
	 * 2^36-1+2^28-1.  That in particular means that if r3 >=	\
	 * 2^36-1, then r3+1 has the 2^36 bit set.  So, if r3+1 has	\
	 * the bit clear, r3 already has the answer we want, if it	\
	 * doesn't, the answer is the low 36 bits of r3+1.  So in all	\
	 * cases the answer is the low 36 bits of (r3 + ((r3+1) >> 36))*/\
	addi	rx,rt,1;						\
	srdi	rx,rx,VSID_BITS_##size;	/* extract 2^VSID_BITS bit */	\
	add	rt,rt,rx

/* 4 bits per slice and we have one slice per 1TB */
#define SLICE_ARRAY_SIZE  (PGTABLE_RANGE >> 41)

#ifndef __ASSEMBLY__

#ifdef CONFIG_PPC_SUBPAGE_PROT
/*
 * For the sub-page protection option, we extend the PGD with one of
 * these.  Basically we have a 3-level tree, with the top level being
 * the protptrs array.  To optimize speed and memory consumption when
 * only addresses < 4GB are being protected, pointers to the first
 * four pages of sub-page protection words are stored in the low_prot
 * array.
 * Each page of sub-page protection words protects 1GB (4 bytes
 * protects 64k).  For the 3-level tree, each page of pointers then
 * protects 8TB.
 */
struct subpage_prot_table {
	unsigned long maxaddr;	/* only addresses < this are protected */
	unsigned int **protptrs[2];
	unsigned int *low_prot[4];
};

#define SBP_L1_BITS		(PAGE_SHIFT - 2)
#define SBP_L2_BITS		(PAGE_SHIFT - 3)
#define SBP_L1_COUNT		(1 << SBP_L1_BITS)
#define SBP_L2_COUNT		(1 << SBP_L2_BITS)
#define SBP_L2_SHIFT		(PAGE_SHIFT + SBP_L1_BITS)
#define SBP_L3_SHIFT		(SBP_L2_SHIFT + SBP_L2_BITS)

extern void subpage_prot_free(struct mm_struct *mm);
extern void subpage_prot_init_new_context(struct mm_struct *mm);
#else
static inline void subpage_prot_free(struct mm_struct *mm) {}
	} else {
		mask = (1ul << (SID_SHIFT_1T - VPN_SHIFT)) - 1;
		vsid = vpn >> (SID_SHIFT_1T - VPN_SHIFT);
		hash = vsid ^ (vsid << 25) ^
			((vpn & mask) >> (shift - VPN_SHIFT)) ;
	}
	return hash & 0x7fffffffffUL;
}

extern int __hash_page_4K(unsigned long ea, unsigned long access,
			  unsigned long vsid, pte_t *ptep, unsigned long trap,
			  unsigned int local, int ssize, int subpage_prot);
extern int __hash_page_64K(unsigned long ea, unsigned long access,
			   unsigned long vsid, pte_t *ptep, unsigned long trap,
			   unsigned int local, int ssize);
struct mm_struct;
unsigned int hash_page_do_lazy_icache(unsigned int pp, pte_t pte, int trap);
extern int hash_page(unsigned long ea, unsigned long access, unsigned long trap);
int __hash_page_huge(unsigned long ea, unsigned long access, unsigned long vsid,
		     pte_t *ptep, unsigned long trap, int local, int ssize,
		     unsigned int shift, unsigned int mmu_psize);
extern void hash_failure_debug(unsigned long ea, unsigned long access,
			       unsigned long vsid, unsigned long trap,
			       int ssize, int psize, unsigned long pte);
extern int htab_bolt_mapping(unsigned long vstart, unsigned long vend,
			     unsigned long pstart, unsigned long prot,
			     int psize, int ssize);
extern void add_gpage(u64 addr, u64 page_size, unsigned long number_of_pages);
extern void demote_segment_4k(struct mm_struct *mm, unsigned long addr);

extern void hpte_init_native(void);
extern void hpte_init_lpar(void);
extern void hpte_init_beat(void);
extern void hpte_init_beat_v3(void);

extern void stabs_alloc(void);
extern void slb_initialize(void);
extern void slb_flush_and_rebolt(void);
extern void stab_initialize(unsigned long stab);

extern void slb_vmalloc_update(void);
extern void slb_set_size(u16 size);
#endif /* __ASSEMBLY__ */

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
 *	VSID = (proto-VSID * VSID_MULTIPLIER) % VSID_MODULUS
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
 * computed is: (protovsid*VSID_MULTIPLIER) % VSID_MODULUS
 *
 *	rt = register continaing the proto-VSID and into which the
 *		VSID will be stored
 *	rx = scratch register (clobbered)
 *
 * 	- rt and rx must be different registers
 * 	- The answer will end up in the low VSID_BITS bits of rt.  The higher
 * 	  bits may contain other garbage, so you may need to mask the
 * 	  result.
 */
#define ASM_VSID_SCRAMBLE(rt, rx, size)					\
	lis	rx,VSID_MULTIPLIER_##size@h;				\
	ori	rx,rx,VSID_MULTIPLIER_##size@l;				\
	mulld	rt,rt,rx;		/* rt = rt * MULTIPLIER */	\
									\
	srdi	rx,rt,VSID_BITS_##size;					\
	clrldi	rt,rt,(64-VSID_BITS_##size);				\
	add	rt,rt,rx;		/* add high and low bits */	\
	/* NOTE: explanation based on VSID_BITS_##size = 36		\
	 * Now, r3 == VSID (mod 2^36-1), and lies between 0 and		\
	 * 2^36-1+2^28-1.  That in particular means that if r3 >=	\
	 * 2^36-1, then r3+1 has the 2^36 bit set.  So, if r3+1 has	\
	 * the bit clear, r3 already has the answer we want, if it	\
	 * doesn't, the answer is the low 36 bits of r3+1.  So in all	\
	 * cases the answer is the low 36 bits of (r3 + ((r3+1) >> 36))*/\
	addi	rx,rt,1;						\
	srdi	rx,rx,VSID_BITS_##size;	/* extract 2^VSID_BITS bit */	\
	add	rt,rt,rx

/* 4 bits per slice and we have one slice per 1TB */
#define SLICE_ARRAY_SIZE  (PGTABLE_RANGE >> 41)

#ifndef __ASSEMBLY__

#ifdef CONFIG_PPC_SUBPAGE_PROT
/*
 * For the sub-page protection option, we extend the PGD with one of
 * these.  Basically we have a 3-level tree, with the top level being
 * the protptrs array.  To optimize speed and memory consumption when
 * only addresses < 4GB are being protected, pointers to the first
 * four pages of sub-page protection words are stored in the low_prot
 * array.
 * Each page of sub-page protection words protects 1GB (4 bytes
 * protects 64k).  For the 3-level tree, each page of pointers then
 * protects 8TB.
 */
struct subpage_prot_table {
	unsigned long maxaddr;	/* only addresses < this are protected */
	unsigned int **protptrs[2];
	unsigned int *low_prot[4];
};

#define SBP_L1_BITS		(PAGE_SHIFT - 2)
#define SBP_L2_BITS		(PAGE_SHIFT - 3)
#define SBP_L1_COUNT		(1 << SBP_L1_BITS)
#define SBP_L2_COUNT		(1 << SBP_L2_BITS)
#define SBP_L2_SHIFT		(PAGE_SHIFT + SBP_L1_BITS)
#define SBP_L3_SHIFT		(SBP_L2_SHIFT + SBP_L2_BITS)

extern void subpage_prot_free(struct mm_struct *mm);
extern void subpage_prot_init_new_context(struct mm_struct *mm);
#else
static inline void subpage_prot_free(struct mm_struct *mm) {}
	} else {
		mask = (1ul << (SID_SHIFT_1T - VPN_SHIFT)) - 1;
		vsid = vpn >> (SID_SHIFT_1T - VPN_SHIFT);
		hash = vsid ^ (vsid << 25) ^
			((vpn & mask) >> (shift - VPN_SHIFT)) ;
	}
	return hash & 0x7fffffffffUL;
}

extern int __hash_page_4K(unsigned long ea, unsigned long access,
			  unsigned long vsid, pte_t *ptep, unsigned long trap,
			  unsigned int local, int ssize, int subpage_prot);
extern int __hash_page_64K(unsigned long ea, unsigned long access,
			   unsigned long vsid, pte_t *ptep, unsigned long trap,
			   unsigned int local, int ssize);
struct mm_struct;
unsigned int hash_page_do_lazy_icache(unsigned int pp, pte_t pte, int trap);
extern int hash_page(unsigned long ea, unsigned long access, unsigned long trap);
int __hash_page_huge(unsigned long ea, unsigned long access, unsigned long vsid,
		     pte_t *ptep, unsigned long trap, int local, int ssize,
		     unsigned int shift, unsigned int mmu_psize);
extern void hash_failure_debug(unsigned long ea, unsigned long access,
			       unsigned long vsid, unsigned long trap,
			       int ssize, int psize, unsigned long pte);
extern int htab_bolt_mapping(unsigned long vstart, unsigned long vend,
			     unsigned long pstart, unsigned long prot,
			     int psize, int ssize);
extern void add_gpage(u64 addr, u64 page_size, unsigned long number_of_pages);
extern void demote_segment_4k(struct mm_struct *mm, unsigned long addr);

extern void hpte_init_native(void);
extern void hpte_init_lpar(void);
extern void hpte_init_beat(void);
extern void hpte_init_beat_v3(void);

extern void stabs_alloc(void);
extern void slb_initialize(void);
extern void slb_flush_and_rebolt(void);
extern void stab_initialize(unsigned long stab);

extern void slb_vmalloc_update(void);
extern void slb_set_size(u16 size);
#endif /* __ASSEMBLY__ */

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
 *	VSID = (proto-VSID * VSID_MULTIPLIER) % VSID_MODULUS
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
 * computed is: (protovsid*VSID_MULTIPLIER) % VSID_MODULUS
 *
 *	rt = register continaing the proto-VSID and into which the
 *		VSID will be stored
 *	rx = scratch register (clobbered)
 *
 * 	- rt and rx must be different registers
 * 	- The answer will end up in the low VSID_BITS bits of rt.  The higher
 * 	  bits may contain other garbage, so you may need to mask the
 * 	  result.
 */
#define ASM_VSID_SCRAMBLE(rt, rx, size)					\
	lis	rx,VSID_MULTIPLIER_##size@h;				\
	ori	rx,rx,VSID_MULTIPLIER_##size@l;				\
	mulld	rt,rt,rx;		/* rt = rt * MULTIPLIER */	\
									\
	srdi	rx,rt,VSID_BITS_##size;					\
	clrldi	rt,rt,(64-VSID_BITS_##size);				\
	add	rt,rt,rx;		/* add high and low bits */	\
	/* NOTE: explanation based on VSID_BITS_##size = 36		\
	 * Now, r3 == VSID (mod 2^36-1), and lies between 0 and		\
	 * 2^36-1+2^28-1.  That in particular means that if r3 >=	\
	 * 2^36-1, then r3+1 has the 2^36 bit set.  So, if r3+1 has	\
	 * the bit clear, r3 already has the answer we want, if it	\
	 * doesn't, the answer is the low 36 bits of r3+1.  So in all	\
	 * cases the answer is the low 36 bits of (r3 + ((r3+1) >> 36))*/\
	addi	rx,rt,1;						\
	srdi	rx,rx,VSID_BITS_##size;	/* extract 2^VSID_BITS bit */	\
	add	rt,rt,rx

/* 4 bits per slice and we have one slice per 1TB */
#define SLICE_ARRAY_SIZE  (PGTABLE_RANGE >> 41)

#ifndef __ASSEMBLY__

#ifdef CONFIG_PPC_SUBPAGE_PROT
/*
 * For the sub-page protection option, we extend the PGD with one of
 * these.  Basically we have a 3-level tree, with the top level being
 * the protptrs array.  To optimize speed and memory consumption when
 * only addresses < 4GB are being protected, pointers to the first
 * four pages of sub-page protection words are stored in the low_prot
 * array.
 * Each page of sub-page protection words protects 1GB (4 bytes
 * protects 64k).  For the 3-level tree, each page of pointers then
 * protects 8TB.
 */
struct subpage_prot_table {
	unsigned long maxaddr;	/* only addresses < this are protected */
	unsigned int **protptrs[2];
	unsigned int *low_prot[4];
};

#define SBP_L1_BITS		(PAGE_SHIFT - 2)
#define SBP_L2_BITS		(PAGE_SHIFT - 3)
#define SBP_L1_COUNT		(1 << SBP_L1_BITS)
#define SBP_L2_COUNT		(1 << SBP_L2_BITS)
#define SBP_L2_SHIFT		(PAGE_SHIFT + SBP_L1_BITS)
#define SBP_L3_SHIFT		(SBP_L2_SHIFT + SBP_L2_BITS)

extern void subpage_prot_free(struct mm_struct *mm);
extern void subpage_prot_init_new_context(struct mm_struct *mm);
#else
static inline void subpage_prot_free(struct mm_struct *mm) {}
typedef struct {
	mm_context_id_t id;
	u16 user_psize;		/* page size index */

#ifdef CONFIG_PPC_MM_SLICES
	u64 low_slices_psize;	/* SLB page size encodings */
	unsigned char high_slices_psize[SLICE_ARRAY_SIZE];
#else
	u16 sllp;		/* SLB page size encoding */
#endif
	unsigned long vdso_base;
#ifdef CONFIG_PPC_SUBPAGE_PROT
	struct subpage_prot_table spt;
#endif /* CONFIG_PPC_SUBPAGE_PROT */
#ifdef CONFIG_PPC_ICSWX
	struct spinlock *cop_lockp; /* guard acop and cop_pid */
	unsigned long acop;	/* mask of enabled coprocessor types */
	unsigned int cop_pid;	/* pid value used with coprocessors */
#endif /* CONFIG_PPC_ICSWX */
} mm_context_t;


#if 0
/*
 * The code below is equivalent to this function for arguments
 * < 2^VSID_BITS, which is all this should ever be called
 * with.  However gcc is not clever enough to compute the
 * modulus (2^n-1) without a second multiply.
 */
#define vsid_scramble(protovsid, size) \
	((((protovsid) * VSID_MULTIPLIER_##size) % VSID_MODULUS_##size))

#else /* 1 */
#define vsid_scramble(protovsid, size) \
	({								 \
		unsigned long x;					 \
		x = (protovsid) * VSID_MULTIPLIER_##size;		 \
		x = (x >> VSID_BITS_##size) + (x & VSID_MODULUS_##size); \
		(x + ((x+1) >> VSID_BITS_##size)) & VSID_MODULUS_##size; \
	})
#endif /* 1 */

/* Returns the segment size indicator for a user address */
static inline int user_segment_size(unsigned long addr)
{
	/* Use 1T segments if possible for addresses >= 1T */
	if (addr >= (1UL << SID_SHIFT_1T))
		return mmu_highuser_ssize;
	return MMU_SEGSIZE_256M;
}
{
	/* Use 1T segments if possible for addresses >= 1T */
	if (addr >= (1UL << SID_SHIFT_1T))
		return mmu_highuser_ssize;
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