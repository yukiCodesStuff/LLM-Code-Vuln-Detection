{
	struct rt_sigframe *frame;
	int err = 0;

	frame = get_sigframe(ksig, regs, sizeof(*frame));

	if (ksig->ka.sa.sa_flags & SA_SIGINFO)
		err |= copy_siginfo_to_user(&frame->info, &ksig->info);

	/* Create the ucontext.  */
	err |= __put_user(0, &frame->uc.uc_flags);
	err |= __put_user(0, &frame->uc.uc_link);
	err |= __save_altstack(&frame->uc.uc_stack, regs->sp);
	err |= rt_setup_ucontext(&frame->uc, regs);
	err |= copy_to_user(&frame->uc.uc_sigmask, set, sizeof(*set));

	if (err)
		goto give_sigsegv;

	/* Set up to return from userspace; jump to fixed address sigreturn
	   trampoline on kuser page.  */
	regs->ra = (unsigned long) (0x1044);

	/* Set up registers for signal handler */
	regs->sp = (unsigned long) frame;
	regs->r4 = (unsigned long) ksig->sig;
	regs->r5 = (unsigned long) &frame->info;
	regs->r6 = (unsigned long) &frame->uc;
	regs->ea = (unsigned long) ksig->ka.sa.sa_handler;
	return 0;

give_sigsegv:
	force_sigsegv(ksig->sig, current);
	return -EFAULT;
}

/*
 * OK, we're invoking a handler
 */
static void handle_signal(struct ksignal *ksig, struct pt_regs *regs)
{
	int ret;
	sigset_t *oldset = sigmask_to_save();

	/* set up the stack frame */
	ret = setup_rt_frame(ksig, oldset, regs);

	signal_setup_done(ret, ksig, 0);
}

static int do_signal(struct pt_regs *regs)
{
	unsigned int retval = 0, continue_addr = 0, restart_addr = 0;
	int restart = 0;
	struct ksignal ksig;

	current->thread.kregs = regs;

	/*
	 * If we were from a system call, check for system call restarting...
	 */
	if (regs->orig_r2 >= 0) {
		continue_addr = regs->ea;
		restart_addr = continue_addr - 4;
		retval = regs->r2;

		/*
		 * Prepare for system call restart. We do this here so that a
		 * debugger will see the already changed PC.
		 */
		switch (retval) {
		case ERESTART_RESTARTBLOCK:
			restart = -2;
		case ERESTARTNOHAND:
		case ERESTARTSYS:
		case ERESTARTNOINTR:
			restart++;
			regs->r2 = regs->orig_r2;
			regs->r7 = regs->orig_r7;
			regs->ea = restart_addr;
			break;
		}
	}