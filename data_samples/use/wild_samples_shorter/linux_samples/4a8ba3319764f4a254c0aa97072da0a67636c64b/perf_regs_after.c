{
	return PERF_SAMPLE_REGS_ABI_32;
}

void perf_get_regs_user(struct perf_regs *regs_user,
			struct pt_regs *regs,
			struct pt_regs *regs_user_copy)
{
	regs_user->regs = task_pt_regs(current);
	regs_user->abi = perf_reg_abi(current);
}
#else /* CONFIG_X86_64 */
#define REG_NOSUPPORT ((1ULL << PERF_REG_X86_DS) | \
		       (1ULL << PERF_REG_X86_ES) | \
		       (1ULL << PERF_REG_X86_FS) | \
	else
		return PERF_SAMPLE_REGS_ABI_64;
}

void perf_get_regs_user(struct perf_regs *regs_user,
			struct pt_regs *regs,
			struct pt_regs *regs_user_copy)
{
	struct pt_regs *user_regs = task_pt_regs(current);

	/*
	 * If we're in an NMI that interrupted task_pt_regs setup, then
	 * we can't sample user regs at all.  This check isn't really
	 * sufficient, though, as we could be in an NMI inside an interrupt
	 * that happened during task_pt_regs setup.
	 */
	if (regs->sp > (unsigned long)&user_regs->r11 &&
	    regs->sp <= (unsigned long)(user_regs + 1)) {
		regs_user->abi = PERF_SAMPLE_REGS_ABI_NONE;
		regs_user->regs = NULL;
		return;
	}

	/*
	 * RIP, flags, and the argument registers are usually saved.
	 * orig_ax is probably okay, too.
	 */
	regs_user_copy->ip = user_regs->ip;
	regs_user_copy->cx = user_regs->cx;
	regs_user_copy->dx = user_regs->dx;
	regs_user_copy->si = user_regs->si;
	regs_user_copy->di = user_regs->di;
	regs_user_copy->r8 = user_regs->r8;
	regs_user_copy->r9 = user_regs->r9;
	regs_user_copy->r10 = user_regs->r10;
	regs_user_copy->r11 = user_regs->r11;
	regs_user_copy->orig_ax = user_regs->orig_ax;
	regs_user_copy->flags = user_regs->flags;

	/*
	 * Don't even try to report the "rest" regs.
	 */
	regs_user_copy->bx = -1;
	regs_user_copy->bp = -1;
	regs_user_copy->r12 = -1;
	regs_user_copy->r13 = -1;
	regs_user_copy->r14 = -1;
	regs_user_copy->r15 = -1;

	/*
	 * For this to be at all useful, we need a reasonable guess for
	 * sp and the ABI.  Be careful: we're in NMI context, and we're
	 * considering current to be the current task, so we should
	 * be careful not to look at any other percpu variables that might
	 * change during context switches.
	 */
	if (IS_ENABLED(CONFIG_IA32_EMULATION) &&
	    task_thread_info(current)->status & TS_COMPAT) {
		/* Easy case: we're in a compat syscall. */
		regs_user->abi = PERF_SAMPLE_REGS_ABI_32;
		regs_user_copy->sp = user_regs->sp;
		regs_user_copy->cs = user_regs->cs;
		regs_user_copy->ss = user_regs->ss;
	} else if (user_regs->orig_ax != -1) {
		/*
		 * We're probably in a 64-bit syscall.
		 * Warning: this code is severely racy.  At least it's better
		 * than just blindly copying user_regs.
		 */
		regs_user->abi = PERF_SAMPLE_REGS_ABI_64;
		regs_user_copy->sp = this_cpu_read(old_rsp);
		regs_user_copy->cs = __USER_CS;
		regs_user_copy->ss = __USER_DS;
		regs_user_copy->cx = -1;  /* usually contains garbage */
	} else {
		/* We're probably in an interrupt or exception. */
		regs_user->abi = user_64bit_mode(user_regs) ?
			PERF_SAMPLE_REGS_ABI_64 : PERF_SAMPLE_REGS_ABI_32;
		regs_user_copy->sp = user_regs->sp;
		regs_user_copy->cs = user_regs->cs;
		regs_user_copy->ss = user_regs->ss;
	}

	regs_user->regs = regs_user_copy;
}
#endif /* CONFIG_X86_32 */