 * No other registers are examined or changed.
 */
_GLOBAL(slb_allocate_realmode)
	/*
	 * check for bad kernel/user address
	 * (ea & ~REGION_MASK) >= PGTABLE_RANGE
	 */
	rldicr. r9,r3,4,(63 - 46 - 4)
	bne-	8f

	srdi	r9,r3,60		/* get region */
	srdi	r10,r3,SID_SHIFT	/* get esid */
	cmpldi	cr7,r9,0xc		/* cmp PAGE_OFFSET for later use */

	/* r3 = address, r10 = esid, cr7 = <> PAGE_OFFSET */
	blt	cr7,0f			/* user or kernel? */
	 */
_GLOBAL(slb_miss_kernel_load_linear)
	li	r11,0
	/*
	 * context = (MAX_USER_CONTEXT) + ((ea >> 60) - 0xc) + 1
	 * r9 = region id.
	 */
	addis	r9,r9,(MAX_USER_CONTEXT - 0xc + 1)@ha
	addi	r9,r9,(MAX_USER_CONTEXT - 0xc + 1)@l


BEGIN_FTR_SECTION
	b	slb_finish_load
END_MMU_FTR_SECTION_IFCLR(MMU_FTR_1T_SEGMENT)
	b	slb_finish_load_1T
	_GLOBAL(slb_miss_kernel_load_io)
	li	r11,0
6:
	/*
	 * context = (MAX_USER_CONTEXT) + ((ea >> 60) - 0xc) + 1
	 * r9 = region id.
	 */
	addis	r9,r9,(MAX_USER_CONTEXT - 0xc + 1)@ha
	addi	r9,r9,(MAX_USER_CONTEXT - 0xc + 1)@l

BEGIN_FTR_SECTION
	b	slb_finish_load
END_MMU_FTR_SECTION_IFCLR(MMU_FTR_1T_SEGMENT)
	b	slb_finish_load_1T

0:
	/* when using slices, we extract the psize off the slice bitmaps
	 * and then we need to get the sllp encoding off the mmu_psize_defs
	 * array.
	 *
	ld	r9,PACACONTEXTID(r13)
BEGIN_FTR_SECTION
	cmpldi	r10,0x1000
	bge	slb_finish_load_1T
END_MMU_FTR_SECTION_IFSET(MMU_FTR_1T_SEGMENT)
	b	slb_finish_load

8:	/* invalid EA */
	li	r10,0			/* BAD_VSID */
	li	r9,0			/* BAD_VSID */
	li	r11,SLB_VSID_USER	/* flags don't much matter */
	b	slb_finish_load

#ifdef __DISABLED__

	/* get context to calculate proto-VSID */
	ld	r9,PACACONTEXTID(r13)
	/* fall through slb_finish_load */

#endif /* __DISABLED__ */

/*
 * Finish loading of an SLB entry and return
 *
 * r3 = EA, r9 = context, r10 = ESID, r11 = flags, clobbers r9, cr7 = <> PAGE_OFFSET
 */
slb_finish_load:
	rldimi  r10,r9,ESID_BITS,0
	ASM_VSID_SCRAMBLE(r10,r9,256M)
	/*
	 * bits above VSID_BITS_256M need to be ignored from r10
	 * also combine VSID and flags
/*
 * Finish loading of a 1T SLB entry (for the kernel linear mapping) and return.
 *
 * r3 = EA, r9 = context, r10 = ESID(256MB), r11 = flags, clobbers r9
 */
slb_finish_load_1T:
	srdi	r10,r10,(SID_SHIFT_1T - SID_SHIFT)	/* get 1T ESID */
	rldimi  r10,r9,ESID_BITS_1T,0
	ASM_VSID_SCRAMBLE(r10,r9,1T)
	/*
	 * bits above VSID_BITS_1T need to be ignored from r10
	 * also combine VSID and flags