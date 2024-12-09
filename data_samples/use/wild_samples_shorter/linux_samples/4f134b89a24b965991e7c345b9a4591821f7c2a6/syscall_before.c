
static int collect_syscall(struct task_struct *target, struct syscall_info *info)
{
	struct pt_regs *regs;

	if (!try_get_task_stack(target)) {
		/* Task has no stack, so the task isn't in a syscall. */

	info->data.nr = syscall_get_nr(target, regs);
	if (info->data.nr != -1L)
		syscall_get_arguments(target, regs,
				      (unsigned long *)&info->data.args[0]);

	put_task_stack(target);
	return 0;
}