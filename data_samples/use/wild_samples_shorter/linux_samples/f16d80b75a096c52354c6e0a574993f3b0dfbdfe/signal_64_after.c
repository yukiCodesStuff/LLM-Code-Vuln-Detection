	if (MSR_TM_ACTIVE(msr)) {
		/* We recheckpoint on return. */
		struct ucontext __user *uc_transact;

		/* Trying to start TM on non TM system */
		if (!cpu_has_feature(CPU_FTR_TM))
			goto badframe;

		if (__get_user(uc_transact, &uc->uc_link))
			goto badframe;
		if (restore_tm_sigcontexts(current, &uc->uc_mcontext,
					   &uc_transact->uc_mcontext))