		.data		= &bpf_jit_enable,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
# ifdef CONFIG_HAVE_EBPF_JIT
	{
		.procname	= "bpf_jit_harden",