	.endm

	.macro	restore_user_regs, fast = 0, offset = 0
	mov	r2, sp
	ldr	r1, [r2, #\offset + S_PSR]	@ get calling cpsr
	ldr	lr, [r2, #\offset + S_PC]!	@ get pc
	msr	spsr_cxsf, r1			@ save in spsr_svc
#if defined(CONFIG_CPU_V6) || defined(CONFIG_CPU_32v6K)
	@ We must avoid clrex due to Cortex-A15 erratum #830321
	strex	r1, r2, [r2]			@ clear the exclusive monitor
#endif
	.if	\fast
	ldmdb	r2, {r1 - lr}^			@ get calling r1 - lr
	.else
	ldmdb	r2, {r0 - lr}^			@ get calling r0 - lr
	.endif
	mov	r0, r0				@ ARMv5T and earlier require a nop
						@ after ldm {}^
	add	sp, sp, #\offset + S_FRAME_SIZE
	movs	pc, lr				@ return & move spsr_svc into cpsr
	.endm

#else	/* CONFIG_THUMB2_KERNEL */