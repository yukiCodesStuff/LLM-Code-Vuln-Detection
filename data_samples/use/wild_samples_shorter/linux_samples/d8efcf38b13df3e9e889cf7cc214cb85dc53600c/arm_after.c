				    unsigned long cmd,
				    void *v)
{
	if (cmd == CPU_PM_EXIT &&
	    __hyp_get_vectors() == hyp_default_vectors) {
		cpu_init_hyp_mode(NULL);
		return NOTIFY_OK;
	}
