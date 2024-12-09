	if (uc_transact) {
		u32 cmcp;
		struct mcontext __user *mcp;

		if (__get_user(cmcp, &uc_transact->uc_regs))
			return -EFAULT;
		mcp = (struct mcontext __user *)(u64)cmcp;
		/* The top 32 bits of the MSR are stashed in the transactional
		 * ucontext. */
		if (__get_user(msr_hi, &mcp->mc_gregs[PT_MSR]))
			goto bad;

		if (MSR_TM_ACTIVE(msr_hi<<32)) {
			/* We only recheckpoint on return if we're
			 * transaction.
			 */
			tm_restore = 1;
			if (do_setcontext_tm(&rt_sf->uc, uc_transact, regs))
				goto bad;
		}