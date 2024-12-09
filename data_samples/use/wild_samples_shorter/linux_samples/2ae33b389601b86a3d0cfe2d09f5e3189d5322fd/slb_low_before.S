 * No other registers are examined or changed.
 */
_GLOBAL(slb_allocate_realmode)
	/* r3 = faulting address */

	srdi	r9,r3,60		/* get region */
	srdi	r10,r3,28		/* get esid */
	cmpldi	cr7,r9,0xc		/* cmp PAGE_OFFSET for later use */

	/* r3 = address, r10 = esid, cr7 = <> PAGE_OFFSET */
	blt	cr7,0f			/* user or kernel? */
	 */
_GLOBAL(slb_miss_kernel_load_linear)
	li	r11,0
	li	r9,0x1
	/*
	 * for 1T we shift 12 bits more.  slb_finish_load_1T will do
	 * the necessary adjustment
	 */
	rldimi  r10,r9,(CONTEXT_BITS + USER_ESID_BITS),0
BEGIN_FTR_SECTION
	b	slb_finish_load
END_MMU_FTR_SECTION_IFCLR(MMU_FTR_1T_SEGMENT)
	b	slb_finish_load_1T
	_GLOBAL(slb_miss_kernel_load_io)
	li	r11,0
6:
	li	r9,0x1
	/*
	 * for 1T we shift 12 bits more.  slb_finish_load_1T will do
	 * the necessary adjustment
	 */
	rldimi  r10,r9,(CONTEXT_BITS + USER_ESID_BITS),0
BEGIN_FTR_SECTION
	b	slb_finish_load
END_MMU_FTR_SECTION_IFCLR(MMU_FTR_1T_SEGMENT)
	b	slb_finish_load_1T

0:	/* user address: proto-VSID = context << 15 | ESID. First check
	 * if the address is within the boundaries of the user region
	 */
	srdi.	r9,r10,USER_ESID_BITS
	bne-	8f			/* invalid ea bits set */


	/* when using slices, we extract the psize off the slice bitmaps
	 * and then we need to get the sllp encoding off the mmu_psize_defs
	 * array.
	 *
	ld	r9,PACACONTEXTID(r13)
BEGIN_FTR_SECTION
	cmpldi	r10,0x1000
END_MMU_FTR_SECTION_IFSET(MMU_FTR_1T_SEGMENT)
	rldimi	r10,r9,USER_ESID_BITS,0
BEGIN_FTR_SECTION
	bge	slb_finish_load_1T
END_MMU_FTR_SECTION_IFSET(MMU_FTR_1T_SEGMENT)
	b	slb_finish_load

8:	/* invalid EA */
	li	r10,0			/* BAD_VSID */
	li	r11,SLB_VSID_USER	/* flags don't much matter */
	b	slb_finish_load

#ifdef __DISABLED__

	/* get context to calculate proto-VSID */
	ld	r9,PACACONTEXTID(r13)
	rldimi	r10,r9,USER_ESID_BITS,0

	/* fall through slb_finish_load */

#endif /* __DISABLED__ */

/*
 * Finish loading of an SLB entry and return
 *
 * r3 = EA, r10 = proto-VSID, r11 = flags, clobbers r9, cr7 = <> PAGE_OFFSET
 */
slb_finish_load:
	ASM_VSID_SCRAMBLE(r10,r9,256M)
	/*
	 * bits above VSID_BITS_256M need to be ignored from r10
	 * also combine VSID and flags
/*
 * Finish loading of a 1T SLB entry (for the kernel linear mapping) and return.
 *
 * r3 = EA, r10 = proto-VSID, r11 = flags, clobbers r9
 */
slb_finish_load_1T:
	srdi	r10,r10,40-28		/* get 1T ESID */
	ASM_VSID_SCRAMBLE(r10,r9,1T)
	/*
	 * bits above VSID_BITS_1T need to be ignored from r10
	 * also combine VSID and flags