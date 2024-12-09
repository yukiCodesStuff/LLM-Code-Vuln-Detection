	mflr	r10 ;						\
	ld	r12,PACAKBASE(r13) ; 				\
	LOAD_HANDLER(r12, system_call_entry_direct) ;		\
	mtctr	r12 ;						\
	mfspr	r12,SPRN_SRR1 ;					\
	/* Re-use of r13... No spare regs to do this */	\
	li	r13,MSR_RI ;					\
	mtmsrd 	r13,1 ;						\
	GET_PACA(r13) ;	/* get r13 back */			\
	bctr ;
#else
	/* We can branch directly */
#define SYSCALL_PSERIES_2_DIRECT				\
	mfspr	r12,SPRN_SRR1 ;					\
_GLOBAL(do_stab_bolted)
	stw	r9,PACA_EXSLB+EX_CCR(r13)	/* save CR in exc. frame */
	std	r11,PACA_EXSLB+EX_SRR0(r13)	/* save SRR0 in exc. frame */
	mfspr	r11,SPRN_DAR			/* ea */

	/*
	 * check for bad kernel/user address
	 * (ea & ~REGION_MASK) >= PGTABLE_RANGE
	 */
	rldicr. r9,r11,4,(63 - 46 - 4)
	li	r9,0	/* VSID = 0 for bad address */
	bne-	0f

	/*
	 * Calculate VSID:
	 * This is the kernel vsid, we take the top for context from
	 * the range. context = (MAX_USER_CONTEXT) + ((ea >> 60) - 0xc) + 1
	 * Here we know that (ea >> 60) == 0xc
	 */
	lis	r9,(MAX_USER_CONTEXT + 1)@ha
	addi	r9,r9,(MAX_USER_CONTEXT + 1)@l

	srdi	r10,r11,SID_SHIFT
	rldimi  r10,r9,ESID_BITS,0 /* proto vsid */
	ASM_VSID_SCRAMBLE(r10, r9, 256M)
	rldic	r9,r10,12,16	/* r9 = vsid << 12 */

0:
	/* Hash to the primary group */
	ld	r10,PACASTABVIRT(r13)
	srdi	r11,r11,SID_SHIFT
	rldimi	r10,r11,7,52	/* r10 = first ste of the group */

	/* Search the primary group for a free entry */
1:	ld	r11,0(r10)	/* Test valid bit of the current ste	*/
	andi.	r11,r11,0x80
	beq	2f