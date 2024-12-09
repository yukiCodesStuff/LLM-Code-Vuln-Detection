	.endm
#endif

#if __LINUX_ARM_ARCH__ < 7
	.macro	dsb, args
	mcr	p15, 0, r0, c7, c10, 4
	.endm

	.macro	isb, args
	mcr	p15, 0, r0, c7, r5, 4
	.endm
#endif

	.macro asm_trace_hardirqs_off, save=1
#if defined(CONFIG_TRACE_IRQFLAGS)
	.if \save
	stmdb   sp!, {r0-r3, ip, lr}