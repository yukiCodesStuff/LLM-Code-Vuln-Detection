		.data		= &bpf_jit_enable,
		.maxlen		= sizeof(int),
		.mode		= 0644,
#ifndef CONFIG_BPF_JIT_ALWAYS_ON
		.proc_handler	= proc_dointvec
#else
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &one,
		.extra2		= &one,
#endif
	},
# ifdef CONFIG_HAVE_EBPF_JIT
	{
		.procname	= "bpf_jit_harden",