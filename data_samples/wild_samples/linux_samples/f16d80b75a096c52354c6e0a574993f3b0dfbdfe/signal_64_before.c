	if (MSR_TM_ACTIVE(msr)) {
		/* We recheckpoint on return. */
		struct ucontext __user *uc_transact;
		if (__get_user(uc_transact, &uc->uc_link))
			goto badframe;
		if (restore_tm_sigcontexts(current, &uc->uc_mcontext,
					   &uc_transact->uc_mcontext))
			goto badframe;
	} else
#endif
	{
		/*
		 * Fall through, for non-TM restore
		 *
		 * Unset MSR[TS] on the thread regs since MSR from user
		 * context does not have MSR active, and recheckpoint was
		 * not called since restore_tm_sigcontexts() was not called
		 * also.
		 *
		 * If not unsetting it, the code can RFID to userspace with
		 * MSR[TS] set, but without CPU in the proper state,
		 * causing a TM bad thing.
		 */
		current->thread.regs->msr &= ~MSR_TS_MASK;
		if (restore_sigcontext(current, NULL, 1, &uc->uc_mcontext))
			goto badframe;
	}