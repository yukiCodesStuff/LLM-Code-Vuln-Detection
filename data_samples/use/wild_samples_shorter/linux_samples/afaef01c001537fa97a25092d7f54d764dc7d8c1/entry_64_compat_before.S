
	/* Opportunistic SYSRET */
sysret32_from_system_call:
	TRACE_IRQS_ON			/* User mode traces as IRQs on. */
	movq	RBX(%rsp), %rbx		/* pt_regs->rbx */
	movq	RBP(%rsp), %rbp		/* pt_regs->rbp */
	movq	EFLAGS(%rsp), %r11	/* pt_regs->flags (in r11) */