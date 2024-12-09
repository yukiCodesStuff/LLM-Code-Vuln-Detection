	if (cpumask_intersects(&debug_err_mask, cpumask_of(cpu))) {
		pr_warning("CPU %d failed to clear debug register pairs\n", cpu);
		return;
	}

	/*
	 * Have a crack at enabling monitor mode. We don't actually need
	 * it yet, but reporting an error early is useful if it fails.
	 */
out_mdbgen:
	if (enable_monitor_mode())
		cpumask_or(&debug_err_mask, &debug_err_mask, cpumask_of(cpu));
}

static int __cpuinit dbg_reset_notify(struct notifier_block *self,
				      unsigned long action, void *cpu)
{
	if ((action & ~CPU_TASKS_FROZEN) == CPU_ONLINE)
		smp_call_function_single((int)cpu, reset_ctrl_regs, NULL, 1);

	return NOTIFY_OK;
}