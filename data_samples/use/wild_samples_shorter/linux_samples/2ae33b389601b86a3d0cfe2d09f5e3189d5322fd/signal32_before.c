			  sigset_t *set, struct pt_regs *regs)
{
	struct compat_rt_sigframe __user *frame;
	compat_stack_t stack;
	int err = 0;

	frame = compat_get_sigframe(ka, regs, sizeof(*frame));
