static struct notifier_block hyp_init_cpu_nb = {
	.notifier_call = hyp_init_cpu_notify,
};

#ifdef CONFIG_CPU_PM
static int hyp_init_cpu_pm_notifier(struct notifier_block *self,
				    unsigned long cmd,
				    void *v)
{
	if (cmd == CPU_PM_EXIT) {
		cpu_init_hyp_mode(NULL);
		return NOTIFY_OK;
	}

	return NOTIFY_DONE;
}