				    unsigned long cmd,
				    void *v)
{
	if (cmd == CPU_PM_EXIT) {
		cpu_init_hyp_mode(NULL);
		return NOTIFY_OK;
	}
