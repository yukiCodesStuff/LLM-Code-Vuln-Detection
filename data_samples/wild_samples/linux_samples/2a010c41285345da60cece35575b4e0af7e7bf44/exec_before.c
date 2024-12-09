	struct open_flags open_exec_flags = {
		.open_flag = O_LARGEFILE | O_RDONLY | __FMODE_EXEC,
		.acc_mode = MAY_EXEC,
		.intent = LOOKUP_OPEN,
		.lookup_flags = LOOKUP_FOLLOW,
	};

	if ((flags & ~(AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH)) != 0)
		return ERR_PTR(-EINVAL);
	if (flags & AT_SYMLINK_NOFOLLOW)
		open_exec_flags.lookup_flags &= ~LOOKUP_FOLLOW;
	if (flags & AT_EMPTY_PATH)
		open_exec_flags.lookup_flags |= LOOKUP_EMPTY;

	file = do_filp_open(fd, name, &open_exec_flags);
	if (IS_ERR(file))
		goto out;

	/*
	 * may_open() has already checked for this, so it should be
	 * impossible to trip now. But we need to be extra cautious
	 * and check again at the very end too.
	 */
	err = -EACCES;
	if (WARN_ON_ONCE(!S_ISREG(file_inode(file)->i_mode) ||
			 path_noexec(&file->f_path)))
		goto exit;

	err = deny_write_access(file);
	if (err)
		goto exit;

out:
	return file;

exit:
	fput(file);
	return ERR_PTR(err);
}

/**
 * open_exec - Open a path name for execution
 *
 * @name: path name to open with the intent of executing it.
 *
 * Returns ERR_PTR on failure or allocated struct file on success.
 *
 * As this is a wrapper for the internal do_open_execat(), callers
 * must call allow_write_access() before fput() on release. Also see
 * do_close_execat().
 */
struct file *open_exec(const char *name)
{
	struct filename *filename = getname_kernel(name);
	struct file *f = ERR_CAST(filename);

	if (!IS_ERR(filename)) {
		f = do_open_execat(AT_FDCWD, filename, 0);
		putname(filename);
	}
	struct open_flags open_exec_flags = {
		.open_flag = O_LARGEFILE | O_RDONLY | __FMODE_EXEC,
		.acc_mode = MAY_EXEC,
		.intent = LOOKUP_OPEN,
		.lookup_flags = LOOKUP_FOLLOW,
	};

	if ((flags & ~(AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH)) != 0)
		return ERR_PTR(-EINVAL);
	if (flags & AT_SYMLINK_NOFOLLOW)
		open_exec_flags.lookup_flags &= ~LOOKUP_FOLLOW;
	if (flags & AT_EMPTY_PATH)
		open_exec_flags.lookup_flags |= LOOKUP_EMPTY;

	file = do_filp_open(fd, name, &open_exec_flags);
	if (IS_ERR(file))
		goto out;

	/*
	 * may_open() has already checked for this, so it should be
	 * impossible to trip now. But we need to be extra cautious
	 * and check again at the very end too.
	 */
	err = -EACCES;
	if (WARN_ON_ONCE(!S_ISREG(file_inode(file)->i_mode) ||
			 path_noexec(&file->f_path)))
		goto exit;

	err = deny_write_access(file);
	if (err)
		goto exit;

out:
	return file;

exit:
	fput(file);
	return ERR_PTR(err);
}

/**
 * open_exec - Open a path name for execution
 *
 * @name: path name to open with the intent of executing it.
 *
 * Returns ERR_PTR on failure or allocated struct file on success.
 *
 * As this is a wrapper for the internal do_open_execat(), callers
 * must call allow_write_access() before fput() on release. Also see
 * do_close_execat().
 */
struct file *open_exec(const char *name)
{
	struct filename *filename = getname_kernel(name);
	struct file *f = ERR_CAST(filename);

	if (!IS_ERR(filename)) {
		f = do_open_execat(AT_FDCWD, filename, 0);
		putname(filename);
	}
	return f;
}
{
	if (mutex_lock_interruptible(&current->signal->cred_guard_mutex))
		return -ERESTARTNOINTR;

	bprm->cred = prepare_exec_creds();
	if (likely(bprm->cred))
		return 0;

	mutex_unlock(&current->signal->cred_guard_mutex);
	return -ENOMEM;
}

/* Matches do_open_execat() */
static void do_close_execat(struct file *file)
{
	if (!file)
		return;
	allow_write_access(file);
	fput(file);
}
	for (depth = 0;; depth++) {
		struct file *exec;
		if (depth > 5)
			return -ELOOP;

		ret = search_binary_handler(bprm);
		if (ret < 0)
			return ret;
		if (!bprm->interpreter)
			break;

		exec = bprm->file;
		bprm->file = bprm->interpreter;
		bprm->interpreter = NULL;

		allow_write_access(exec);
		if (unlikely(bprm->have_execfd)) {
			if (bprm->executable) {
				fput(exec);
				return -ENOEXEC;
			}
			bprm->executable = exec;
		} else
			fput(exec);
	}