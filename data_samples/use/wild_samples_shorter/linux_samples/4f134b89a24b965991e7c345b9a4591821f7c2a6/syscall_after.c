
static int collect_syscall(struct task_struct *target, struct syscall_info *info)
{
	unsigned long args[6] = { };
	struct pt_regs *regs;

	if (!try_get_task_stack(target)) {
		/* Task has no stack, so the task isn't in a syscall. */

	info->data.nr = syscall_get_nr(target, regs);
	if (info->data.nr != -1L)
		syscall_get_arguments(target, regs, args);

	info->data.args[0] = args[0];
	info->data.args[1] = args[1];
	info->data.args[2] = args[2];
	info->data.args[3] = args[3];
	info->data.args[4] = args[4];
	info->data.args[5] = args[5];

	put_task_stack(target);
	return 0;
}