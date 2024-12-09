
	.align	2

ENTRY(kvm_call_hyp)
	hvc	#0
	ret
ENDPROC(kvm_call_hyp)
	pop	x2, x3
	pop	x0, x1

	push	lr, xzr

	/*
	 * Compute the function address in EL2, and shuffle the parameters.
	 */
	blr	lr

	pop	lr, xzr
	eret

el1_trap:
	/*
	 * x1: ESR