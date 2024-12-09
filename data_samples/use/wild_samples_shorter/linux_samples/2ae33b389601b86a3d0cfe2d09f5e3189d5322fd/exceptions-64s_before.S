	mflr	r10 ;						\
	ld	r12,PACAKBASE(r13) ; 				\
	LOAD_HANDLER(r12, system_call_entry_direct) ;		\
	mtlr	r12 ;						\
	mfspr	r12,SPRN_SRR1 ;					\
	/* Re-use of r13... No spare regs to do this */	\
	li	r13,MSR_RI ;					\
	mtmsrd 	r13,1 ;						\
	GET_PACA(r13) ;	/* get r13 back */			\
	blr ;
#else
	/* We can branch directly */
#define SYSCALL_PSERIES_2_DIRECT				\
	mfspr	r12,SPRN_SRR1 ;					\
_GLOBAL(do_stab_bolted)
	stw	r9,PACA_EXSLB+EX_CCR(r13)	/* save CR in exc. frame */
	std	r11,PACA_EXSLB+EX_SRR0(r13)	/* save SRR0 in exc. frame */

	/* Hash to the primary group */
	ld	r10,PACASTABVIRT(r13)
	mfspr	r11,SPRN_DAR
	srdi	r11,r11,28
	rldimi	r10,r11,7,52	/* r10 = first ste of the group */

	/* Calculate VSID */
	/* This is a kernel address, so protovsid = ESID | 1 << 37 */
	li	r9,0x1
	rldimi  r11,r9,(CONTEXT_BITS + USER_ESID_BITS),0
	ASM_VSID_SCRAMBLE(r11, r9, 256M)
	rldic	r9,r11,12,16	/* r9 = vsid << 12 */

	/* Search the primary group for a free entry */
1:	ld	r11,0(r10)	/* Test valid bit of the current ste	*/
	andi.	r11,r11,0x80
	beq	2f