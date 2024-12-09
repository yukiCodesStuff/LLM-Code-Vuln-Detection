	if (!infop)
		return err;

	if (!access_ok(VERIFY_WRITE, infop, sizeof(*infop)))
		goto Efault;

	user_access_begin();
	unsafe_put_user(signo, &infop->si_signo, Efault);
	unsafe_put_user(0, &infop->si_errno, Efault);
	unsafe_put_user(info.cause, &infop->si_code, Efault);
	if (!infop)
		return err;

	if (!access_ok(VERIFY_WRITE, infop, sizeof(*infop)))
		goto Efault;

	user_access_begin();
	unsafe_put_user(signo, &infop->si_signo, Efault);
	unsafe_put_user(0, &infop->si_errno, Efault);
	unsafe_put_user(info.cause, &infop->si_code, Efault);