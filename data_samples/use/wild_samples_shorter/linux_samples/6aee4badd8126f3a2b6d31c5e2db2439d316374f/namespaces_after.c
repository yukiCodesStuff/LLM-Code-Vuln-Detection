	const struct proc_ns_operations *ns_ops = PROC_I(inode)->ns_ops;
	struct task_struct *task;
	struct path ns_path;
	int error = -EACCES;

	if (!dentry)
		return ERR_PTR(-ECHILD);

	task = get_proc_task(inode);
	if (!task)
		return ERR_PTR(-EACCES);

	if (!ptrace_may_access(task, PTRACE_MODE_READ_FSCREDS))
		goto out;

	error = ns_get_path(&ns_path, task, ns_ops);
	if (error)
		goto out;

	error = nd_jump_link(&ns_path);
out:
	put_task_struct(task);
	return ERR_PTR(error);
}

static int proc_ns_readlink(struct dentry *dentry, char __user *buffer, int buflen)
{