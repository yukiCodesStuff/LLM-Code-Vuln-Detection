{
	const struct proc_ns_operations *ns_ops = PROC_I(inode)->ns_ops;
	struct task_struct *task;
	struct path ns_path;
	void *error = ERR_PTR(-EACCES);

	if (!dentry)
		return ERR_PTR(-ECHILD);

	task = get_proc_task(inode);
	if (!task)
		return error;

	if (ptrace_may_access(task, PTRACE_MODE_READ_FSCREDS)) {
		error = ns_get_path(&ns_path, task, ns_ops);
		if (!error)
			nd_jump_link(&ns_path);
	}