{
	struct compat_rt_sigframe __user *frame;
	int err = 0;

	frame = compat_get_sigframe(ka, regs, sizeof(*frame));

	if (!frame)
		return 1;

	err |= copy_siginfo_to_user32(&frame->info, info);

	__put_user_error(0, &frame->sig.uc.uc_flags, err);
	__put_user_error(0, &frame->sig.uc.uc_link, err);

	err |= __compat_save_altstack(&frame->sig.uc.uc_stack, regs->compat_sp);

	err |= compat_setup_sigframe(&frame->sig, regs, set);

	if (err == 0) {
		compat_setup_return(regs, ka, frame->sig.retcode, frame, usig);
		regs->regs[1] = (compat_ulong_t)(unsigned long)&frame->info;
		regs->regs[2] = (compat_ulong_t)(unsigned long)&frame->sig.uc;
	}

	return err;
}