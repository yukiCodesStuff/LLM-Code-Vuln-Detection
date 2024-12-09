	.endm
#endif

	.macro asm_trace_hardirqs_off, save=1
#if defined(CONFIG_TRACE_IRQFLAGS)
	.if \save
	stmdb   sp!, {r0-r3, ip, lr}