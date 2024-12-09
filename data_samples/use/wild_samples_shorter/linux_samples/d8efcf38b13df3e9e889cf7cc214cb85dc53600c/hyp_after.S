
	.align	2

/*
 * u64 kvm_call_hyp(void *hypfn, ...);
 *
 * This is not really a variadic function in the classic C-way and care must
 * be taken when calling this to ensure parameters are passed in registers
 * only, since the stack will change between the caller and the callee.
 *
 * Call the function with the first argument containing a pointer to the
 * function you wish to call in Hyp mode, and subsequent arguments will be
 * passed as x0, x1, and x2 (a maximum of 3 arguments in addition to the
 * function pointer can be passed).  The function being called must be mapped
 * in Hyp mode (see init_hyp_mode in arch/arm/kvm/arm.c).  Return values are
 * passed in r0 and r1.
 *
 * A function pointer with a value of 0 has a special meaning, and is
 * used to implement __hyp_get_vectors in the same way as in
 * arch/arm64/kernel/hyp_stub.S.
 */
ENTRY(kvm_call_hyp)
	hvc	#0
	ret
ENDPROC(kvm_call_hyp)
	pop	x2, x3
	pop	x0, x1

	/* Check for __hyp_get_vectors */
	cbnz	x0, 1f
	mrs	x0, vbar_el2
	b	2f

1:	push	lr, xzr

	/*
	 * Compute the function address in EL2, and shuffle the parameters.
	 */
	blr	lr

	pop	lr, xzr
2:	eret

el1_trap:
	/*
	 * x1: ESR