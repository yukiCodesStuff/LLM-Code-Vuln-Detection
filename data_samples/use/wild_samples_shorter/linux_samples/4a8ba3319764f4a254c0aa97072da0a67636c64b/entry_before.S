	GET_THREAD_INFO	r1
	ldw	r4, TI_PREEMPT_COUNT(r1)
	bne	r4, r0, restore_all

need_resched:
	ldw	r4, TI_FLAGS(r1)		/* ? Need resched set */
	BTBZ	r10, r4, TIF_NEED_RESCHED, restore_all
	ldw	r4, PT_ESTATUS(sp)	/* ? Interrupts off */
	andi	r10, r4, ESTATUS_EPIE
	beq	r10, r0, restore_all
	movia	r4, PREEMPT_ACTIVE
	stw	r4, TI_PREEMPT_COUNT(r1)
	rdctl	r10, status		/* enable intrs again */
	ori	r10, r10 ,STATUS_PIE
	wrctl	status, r10
	PUSH	r1
	call	schedule
	POP	r1
	mov	r4, r0
	stw	r4, TI_PREEMPT_COUNT(r1)
	rdctl	r10, status		/* disable intrs */
	andi	r10, r10, %lo(~STATUS_PIE)
	wrctl	status, r10
	br	need_resched
#else
	br	restore_all
#endif

/***********************************************************************
 * A few syscall wrappers
 ***********************************************************************