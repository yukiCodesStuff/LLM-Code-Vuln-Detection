	int retval = 0;
	int flag = 0;
	int ispipe;
	static atomic_t core_dump_count = ATOMIC_INIT(0);
	struct coredump_params cprm = {
		.signr = signr,
		.regs = regs,
	if (!cred)
		goto fail;
	/*
	 *	We cannot trust fsuid as being the "true" uid of the
	 *	process nor do we know its entire history. We only know it
	 *	was tainted so we dump it as root in mode 2.
	 */
	if (__get_dumpable(cprm.mm_flags) == 2) {
		/* Setuid core dump mode */
		flag = O_EXCL;		/* Stop rewrite attacks */
		cred->fsuid = GLOBAL_ROOT_UID;	/* Dump root private */
	}

	retval = coredump_wait(exit_code, &core_state);
	if (retval < 0)
		if (cprm.limit < binfmt->min_coredump)
			goto fail_unlock;

		cprm.file = filp_open(cn.corename,
				 O_CREAT | 2 | O_NOFOLLOW | O_LARGEFILE | flag,
				 0600);
		if (IS_ERR(cprm.file))