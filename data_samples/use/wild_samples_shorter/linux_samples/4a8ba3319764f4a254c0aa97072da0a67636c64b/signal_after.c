
	/* Set up to return from userspace; jump to fixed address sigreturn
	   trampoline on kuser page.  */
	regs->ra = (unsigned long) (0x1044);

	/* Set up registers for signal handler */
	regs->sp = (unsigned long) frame;
	regs->r4 = (unsigned long) ksig->sig;