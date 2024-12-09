	{
		.procname	= "bpf_jit_enable",
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
		.data		= &bpf_jit_harden,
		.maxlen		= sizeof(int),
		.mode		= 0600,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bpf_jit_kallsyms",
		.data		= &bpf_jit_kallsyms,
		.maxlen		= sizeof(int),
		.mode		= 0600,
		.proc_handler	= proc_dointvec,
	},
# endif
#endif
	{
		.procname	= "netdev_tstamp_prequeue",
		.data		= &netdev_tstamp_prequeue,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "message_cost",
		.data		= &net_ratelimit_state.interval,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "message_burst",
		.data		= &net_ratelimit_state.burst,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "optmem_max",
		.data		= &sysctl_optmem_max,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tstamp_allow_data",
		.data		= &sysctl_tstamp_allow_data,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &one
	},
#ifdef CONFIG_RPS
	{
		.procname	= "rps_sock_flow_entries",
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= rps_sock_flow_sysctl
	},
#endif
#ifdef CONFIG_NET_FLOW_LIMIT
	{
		.procname	= "flow_limit_cpu_bitmap",
		.mode		= 0644,
		.proc_handler	= flow_limit_cpu_sysctl
	},
	{
		.procname	= "flow_limit_table_len",
		.data		= &netdev_flow_limit_table_len,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= flow_limit_table_len_sysctl
	},
#endif /* CONFIG_NET_FLOW_LIMIT */
#ifdef CONFIG_NET_RX_BUSY_POLL
	{
		.procname	= "busy_poll",
		.data		= &sysctl_net_busy_poll,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
	},
	{
		.procname	= "busy_read",
		.data		= &sysctl_net_busy_read,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
	},
#endif
#ifdef CONFIG_NET_SCHED
	{
		.procname	= "default_qdisc",
		.mode		= 0644,
		.maxlen		= IFNAMSIZ,
		.proc_handler	= set_default_qdisc
	},
#endif
#endif /* CONFIG_NET */
	{
		.procname	= "netdev_budget",
		.data		= &netdev_budget,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "warnings",
		.data		= &net_msg_warn,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "max_skb_frags",
		.data		= &sysctl_max_skb_frags,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &one,
		.extra2		= &max_skb_frags,
	},
	{
		.procname	= "netdev_budget_usecs",
		.data		= &netdev_budget_usecs,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
	},
	{ }
};

static struct ctl_table netns_core_table[] = {
	{
		.procname	= "somaxconn",
		.data		= &init_net.core.sysctl_somaxconn,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.extra1		= &zero,
		.proc_handler	= proc_dointvec_minmax
	},
	{ }
};

static __net_init int sysctl_core_net_init(struct net *net)
{
	struct ctl_table *tbl;

	tbl = netns_core_table;
	if (!net_eq(net, &init_net)) {
		tbl = kmemdup(tbl, sizeof(netns_core_table), GFP_KERNEL);
		if (tbl == NULL)
			goto err_dup;

		tbl[0].data = &net->core.sysctl_somaxconn;

		/* Don't export any sysctls to unprivileged users */
		if (net->user_ns != &init_user_ns) {
			tbl[0].procname = NULL;
		}
	}

	net->core.sysctl_hdr = register_net_sysctl(net, "net/core", tbl);
	if (net->core.sysctl_hdr == NULL)
		goto err_reg;

	return 0;

err_reg:
	if (tbl != netns_core_table)
		kfree(tbl);
err_dup:
	return -ENOMEM;
}

static __net_exit void sysctl_core_net_exit(struct net *net)
{
	struct ctl_table *tbl;

	tbl = net->core.sysctl_hdr->ctl_table_arg;
	unregister_net_sysctl_table(net->core.sysctl_hdr);
	BUG_ON(tbl == netns_core_table);
	kfree(tbl);
}

static __net_initdata struct pernet_operations sysctl_core_ops = {
	.init = sysctl_core_net_init,
	.exit = sysctl_core_net_exit,
};

static __init int sysctl_core_init(void)
{
	register_net_sysctl(&init_net, "net/core", net_core_table);
	return register_pernet_subsys(&sysctl_core_ops);
}

fs_initcall(sysctl_core_init);