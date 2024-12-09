 * in Hyp mode (see init_hyp_mode in arch/arm/kvm/arm.c).  Return values are
 * passed in r0 and r1.
 *
 * A function pointer with a value of 0xffffffff has a special meaning,
 * and is used to implement __hyp_get_vectors in the same way as in
 * arch/arm/kernel/hyp_stub.S.
 *
 * The calling convention follows the standard AAPCS:
 *   r0 - r3: caller save
 *   r12:     caller save
 *   rest:    callee save
host_switch_to_hyp:
	pop	{r0, r1, r2}

	/* Check for __hyp_get_vectors */
	cmp	r0, #-1
	mrceq	p15, 4, r0, c12, c0, 0	@ get HVBAR
	beq	1f

	push	{lr}
	mrs	lr, SPSR
	push	{lr}

	pop	{lr}
	msr	SPSR_csxf, lr
	pop	{lr}
1:	eret

guest_trap:
	load_vcpu			@ Load VCPU pointer to r0
	str	r1, [vcpu, #VCPU_HSR]