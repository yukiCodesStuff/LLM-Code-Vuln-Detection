static int __cpuinit dbg_reset_notify(struct notifier_block *self,
				      unsigned long action, void *cpu)
{
	if ((action & ~CPU_TASKS_FROZEN) == CPU_ONLINE)
		smp_call_function_single((int)cpu, reset_ctrl_regs, NULL, 1);

	return NOTIFY_OK;
}