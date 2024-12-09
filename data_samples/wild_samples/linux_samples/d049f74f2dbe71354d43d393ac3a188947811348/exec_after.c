{
	int ret;

	ret = mm_flags & MMF_DUMPABLE_MASK;
	return (ret > SUID_DUMP_USER) ? SUID_DUMP_ROOT : ret;
}

/*
 * This returns the actual value of the suid_dumpable flag. For things
 * that are using this for checking for privilege transitions, it must
 * test against SUID_DUMP_USER rather than treating it as a boolean
 * value.
 */
int get_dumpable(struct mm_struct *mm)
{
	return __get_dumpable(mm->flags);
}

SYSCALL_DEFINE3(execve,
		const char __user *, filename,
		const char __user *const __user *, argv,
		const char __user *const __user *, envp)
{
	struct filename *path = getname(filename);
	int error = PTR_ERR(path);
	if (!IS_ERR(path)) {
		error = do_execve(path->name, argv, envp);
		putname(path);
	}