	seq_puts(p, "  Machine check polls\n");
#endif
#if IS_ENABLED(CONFIG_HYPERV) || defined(CONFIG_XEN)
	seq_printf(p, "%*s: ", prec, "HYP");
	for_each_online_cpu(j)
		seq_printf(p, "%10u ", irq_stats(j)->irq_hv_callback_count);
	seq_puts(p, "  Hypervisor callback interrupts\n");
#endif