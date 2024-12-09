{
	struct core_state core_state;
	struct core_name cn;
	struct mm_struct *mm = current->mm;
	struct linux_binfmt * binfmt;
	const struct cred *old_cred;
	struct cred *cred;
	int retval = 0;
	int flag = 0;
	int ispipe;
	bool need_nonrelative = false;
	static atomic_t core_dump_count = ATOMIC_INIT(0);
	struct coredump_params cprm = {
		.signr = signr,
		.regs = regs,
		.limit = rlimit(RLIMIT_CORE),
		/*
		 * We must use the same mm->flags while dumping core to avoid
		 * inconsistency of bit flags, since this flag is not protected
		 * by any locks.
		 */
		.mm_flags = mm->flags,
	};

	audit_core_dumps(signr);

	binfmt = mm->binfmt;
	if (!binfmt || !binfmt->core_dump)
		goto fail;
	if (!__get_dumpable(cprm.mm_flags))
		goto fail;

	cred = prepare_creds();
	if (!cred)
		goto fail;
	/*
	 * We cannot trust fsuid as being the "true" uid of the process
	 * nor do we know its entire history. We only know it was tainted
	 * so we dump it as root in mode 2, and only into a controlled
	 * environment (pipe handler or fully qualified path).
	 */
	if (__get_dumpable(cprm.mm_flags) == 2) {
		/* Setuid core dump mode */
		flag = O_EXCL;		/* Stop rewrite attacks */
		cred->fsuid = GLOBAL_ROOT_UID;	/* Dump root private */
		need_nonrelative = true;
	}

	retval = coredump_wait(exit_code, &core_state);
	if (retval < 0)
		goto fail_creds;

	old_cred = override_creds(cred);

	/*
	 * Clear any false indication of pending signals that might
	 * be seen by the filesystem code called to write the core file.
	 */
	clear_thread_flag(TIF_SIGPENDING);

	ispipe = format_corename(&cn, signr);

 	if (ispipe) {
		int dump_count;
		char **helper_argv;

		if (ispipe < 0) {
			printk(KERN_WARNING "format_corename failed\n");
			printk(KERN_WARNING "Aborting core\n");
			goto fail_corename;
		}

		if (cprm.limit == 1) {
			/*
			 * Normally core limits are irrelevant to pipes, since
			 * we're not writing to the file system, but we use
			 * cprm.limit of 1 here as a speacial value. Any
			 * non-1 limit gets set to RLIM_INFINITY below, but
			 * a limit of 0 skips the dump.  This is a consistent
			 * way to catch recursive crashes.  We can still crash
			 * if the core_pattern binary sets RLIM_CORE =  !1
			 * but it runs as root, and can do lots of stupid things
			 * Note that we use task_tgid_vnr here to grab the pid
			 * of the process group leader.  That way we get the
			 * right pid if a thread in a multi-threaded
			 * core_pattern process dies.
			 */
			printk(KERN_WARNING
				"Process %d(%s) has RLIMIT_CORE set to 1\n",
				task_tgid_vnr(current), current->comm);
			printk(KERN_WARNING "Aborting core\n");
			goto fail_unlock;
		}
		cprm.limit = RLIM_INFINITY;

		dump_count = atomic_inc_return(&core_dump_count);
		if (core_pipe_limit && (core_pipe_limit < dump_count)) {
			printk(KERN_WARNING "Pid %d(%s) over core_pipe_limit\n",
			       task_tgid_vnr(current), current->comm);
			printk(KERN_WARNING "Skipping core dump\n");
			goto fail_dropcount;
		}

		helper_argv = argv_split(GFP_KERNEL, cn.corename+1, NULL);
		if (!helper_argv) {
			printk(KERN_WARNING "%s failed to allocate memory\n",
			       __func__);
			goto fail_dropcount;
		}

		retval = call_usermodehelper_fns(helper_argv[0], helper_argv,
					NULL, UMH_WAIT_EXEC, umh_pipe_setup,
					NULL, &cprm);
		argv_free(helper_argv);
		if (retval) {
 			printk(KERN_INFO "Core dump to %s pipe failed\n",
			       cn.corename);
			goto close_fail;
 		}
	} else {
		struct inode *inode;

		if (cprm.limit < binfmt->min_coredump)
			goto fail_unlock;

		if (need_nonrelative && cn.corename[0] != '/') {
			printk(KERN_WARNING "Pid %d(%s) can only dump core "\
				"to fully qualified path!\n",
				task_tgid_vnr(current), current->comm);
			printk(KERN_WARNING "Skipping core dump\n");
			goto fail_unlock;
		}

		cprm.file = filp_open(cn.corename,
				 O_CREAT | 2 | O_NOFOLLOW | O_LARGEFILE | flag,
				 0600);
		if (IS_ERR(cprm.file))
			goto fail_unlock;

		inode = cprm.file->f_path.dentry->d_inode;
		if (inode->i_nlink > 1)
			goto close_fail;
		if (d_unhashed(cprm.file->f_path.dentry))
			goto close_fail;
		/*
		 * AK: actually i see no reason to not allow this for named
		 * pipes etc, but keep the previous behaviour for now.
		 */
		if (!S_ISREG(inode->i_mode))
			goto close_fail;
		/*
		 * Dont allow local users get cute and trick others to coredump
		 * into their pre-created files.
		 */
		if (!uid_eq(inode->i_uid, current_fsuid()))
			goto close_fail;
		if (!cprm.file->f_op || !cprm.file->f_op->write)
			goto close_fail;
		if (do_truncate(cprm.file->f_path.dentry, 0, 0, cprm.file))
			goto close_fail;
	}

	retval = binfmt->core_dump(&cprm);
	if (retval)
		current->signal->group_exit_code |= 0x80;

	if (ispipe && core_pipe_limit)
		wait_for_dump_helpers(cprm.file);
close_fail:
	if (cprm.file)
		filp_close(cprm.file, NULL);
fail_dropcount:
	if (ispipe)
		atomic_dec(&core_dump_count);
fail_unlock:
	kfree(cn.corename);
fail_corename:
	coredump_finish(mm);
	revert_creds(old_cred);
fail_creds:
	put_cred(cred);
fail:
	return;
}

/*
 * Core dumping helper functions.  These are the only things you should
 * do on a core-file: use only these functions to write out all the
 * necessary info.
 */
int dump_write(struct file *file, const void *addr, int nr)
{
	return access_ok(VERIFY_READ, addr, nr) && file->f_op->write(file, addr, nr, &file->f_pos) == nr;
}
EXPORT_SYMBOL(dump_write);

int dump_seek(struct file *file, loff_t off)
{
	int ret = 1;

	if (file->f_op->llseek && file->f_op->llseek != no_llseek) {
		if (file->f_op->llseek(file, off, SEEK_CUR) < 0)
			return 0;
	} else {
		char *buf = (char *)get_zeroed_page(GFP_KERNEL);

		if (!buf)
			return 0;
		while (off > 0) {
			unsigned long n = off;

			if (n > PAGE_SIZE)
				n = PAGE_SIZE;
			if (!dump_write(file, buf, n)) {
				ret = 0;
				break;
			}
			off -= n;
		}
		free_page((unsigned long)buf);
	}
	return ret;
}
EXPORT_SYMBOL(dump_seek);
	struct coredump_params cprm = {
		.signr = signr,
		.regs = regs,
		.limit = rlimit(RLIMIT_CORE),
		/*
		 * We must use the same mm->flags while dumping core to avoid
		 * inconsistency of bit flags, since this flag is not protected
		 * by any locks.
		 */
		.mm_flags = mm->flags,
	};

	audit_core_dumps(signr);

	binfmt = mm->binfmt;
	if (!binfmt || !binfmt->core_dump)
		goto fail;
	if (!__get_dumpable(cprm.mm_flags))
		goto fail;

	cred = prepare_creds();
	if (!cred)
		goto fail;
	/*
	 * We cannot trust fsuid as being the "true" uid of the process
	 * nor do we know its entire history. We only know it was tainted
	 * so we dump it as root in mode 2, and only into a controlled
	 * environment (pipe handler or fully qualified path).
	 */
	if (__get_dumpable(cprm.mm_flags) == 2) {
		/* Setuid core dump mode */
		flag = O_EXCL;		/* Stop rewrite attacks */
		cred->fsuid = GLOBAL_ROOT_UID;	/* Dump root private */
		need_nonrelative = true;
	}
	} else {
		struct inode *inode;

		if (cprm.limit < binfmt->min_coredump)
			goto fail_unlock;

		if (need_nonrelative && cn.corename[0] != '/') {
			printk(KERN_WARNING "Pid %d(%s) can only dump core "\
				"to fully qualified path!\n",
				task_tgid_vnr(current), current->comm);
			printk(KERN_WARNING "Skipping core dump\n");
			goto fail_unlock;
		}