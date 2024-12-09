 * in Hyp mode (see init_hyp_mode in arch/arm/kvm/arm.c).  Return values are
 * passed in r0 and r1.
 *
 * The calling convention follows the standard AAPCS:
 *   r0 - r3: caller save
 *   r12:     caller save
 *   rest:    callee save
host_switch_to_hyp:
	pop	{r0, r1, r2}

	push	{lr}
	mrs	lr, SPSR
	push	{lr}

	pop	{lr}
	msr	SPSR_csxf, lr
	pop	{lr}
	eret

guest_trap:
	load_vcpu			@ Load VCPU pointer to r0
	str	r1, [vcpu, #VCPU_HSR]