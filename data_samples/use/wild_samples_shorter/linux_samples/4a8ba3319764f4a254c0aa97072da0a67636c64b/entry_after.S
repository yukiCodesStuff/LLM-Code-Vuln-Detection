	GET_THREAD_INFO	r1
	ldw	r4, TI_PREEMPT_COUNT(r1)
	bne	r4, r0, restore_all
	ldw	r4, TI_FLAGS(r1)		/* ? Need resched set */
	BTBZ	r10, r4, TIF_NEED_RESCHED, restore_all
	ldw	r4, PT_ESTATUS(sp)	/* ? Interrupts off */
	andi	r10, r4, ESTATUS_EPIE
	beq	r10, r0, restore_all
	call	preempt_schedule_irq
#endif
	br	restore_all

/***********************************************************************
 * A few syscall wrappers
 ***********************************************************************