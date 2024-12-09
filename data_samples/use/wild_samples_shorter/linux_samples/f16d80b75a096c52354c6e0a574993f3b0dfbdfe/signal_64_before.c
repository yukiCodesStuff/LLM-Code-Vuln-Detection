	if (MSR_TM_ACTIVE(msr)) {
		/* We recheckpoint on return. */
		struct ucontext __user *uc_transact;
		if (__get_user(uc_transact, &uc->uc_link))
			goto badframe;
		if (restore_tm_sigcontexts(current, &uc->uc_mcontext,
					   &uc_transact->uc_mcontext))