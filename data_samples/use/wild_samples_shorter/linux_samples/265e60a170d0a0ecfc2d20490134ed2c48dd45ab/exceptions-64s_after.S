EXC_VIRT(program_check, 0x4700, 0x100, 0x700)
TRAMP_KVM(PACA_EXGEN, 0x700)
EXC_COMMON_BEGIN(program_check_common)
	/*
	 * It's possible to receive a TM Bad Thing type program check with
	 * userspace register values (in particular r1), but with SRR1 reporting
	 * that we came from the kernel. Normally that would confuse the bad
	 * stack logic, and we would report a bad kernel stack pointer. Instead
	 * we switch to the emergency stack if we're taking a TM Bad Thing from
	 * the kernel.
	 */
	li	r10,MSR_PR		/* Build a mask of MSR_PR ..	*/
	oris	r10,r10,0x200000@h	/* .. and SRR1_PROGTM		*/
	and	r10,r10,r12		/* Mask SRR1 with that.		*/
	srdi	r10,r10,8		/* Shift it so we can compare	*/
	cmpldi	r10,(0x200000 >> 8)	/* .. with an immediate.	*/
	bne 1f				/* If != go to normal path.	*/

	/* SRR1 had PR=0 and SRR1_PROGTM=1, so use the emergency stack	*/
	andi.	r10,r12,MSR_PR;		/* Set CR0 correctly for label	*/
					/* 3 in EXCEPTION_PROLOG_COMMON	*/
	mr	r10,r1			/* Save r1			*/
	ld	r1,PACAEMERGSP(r13)	/* Use emergency stack		*/
	subi	r1,r1,INT_FRAME_SIZE	/* alloc stack frame		*/
	b 3f				/* Jump into the macro !!	*/
1:	EXCEPTION_PROLOG_COMMON(0x700, PACA_EXGEN)
	bl	save_nvgprs
	RECONCILE_IRQ_STATE(r10, r11)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	bl	program_check_exception